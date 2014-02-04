#ifndef IO_H
#define IO_H my

#include "std.h"
#include "str.h"
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <string>
#include <thread>

#ifdef __unix__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <poll.h>
#endif
#ifdef _WIN32
#include <inaddr.h>
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#include <curl/curl.h>

NS(IO_H)

static uint available(int so)
{
	int n = 0;
	return ioctl(so,SIOCINQ,&n) == -1 ? 0 : n;
}

static string recv(int fd)
{
	if(fd==-1) return string();
	string ret;
	for (ssize_t x = ret.size();;)
	{
		const ssize_t nn = 4 * 1024;
		ret.resize(ret.size() + nn); /// available
		ssize_t n = ::recv(fd, (char*) ret.c_str() + x, nn, MSG_DONTWAIT);
		if (n == -1) n = 0;
		ret.resize(ret.size() - nn + n);
		if (n == 0 || n < nn) break;
		x += n;
	}
	return ret;
}

static string read(int fd)
{
	string ret;
	if(fd==-1) return ret;
	#if 0
	int fd1 = dup(fd);
	if(fd1==-1) return ret; // dup
	__gnu_cxx::stdio_filebuf<char> filebuf(fd1, std::ios::in);
	std::istream is(&filebuf);
	std::stringbuf b;
	is >> &b;
	return b.str();
	#endif
	// fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK); or ~O_NONBLOCK
	for (ssize_t x = ret.size();;)
	{
		const ssize_t nn = 4 * 1024;
		ret.resize(ret.size() + nn);
		ssize_t n = ::read(fd, (char*) ret.c_str() + x, nn);
		if (n == -1) n = 0;
		ret.resize(ret.size() - nn + n);
		if (n == 0 || n < nn) break;
		x += n;
	}
	return ret;
}

static struct stat stat(const string &path)
{
#ifdef TRACE_H
	plog("stat: %s",path.c_str());
#endif
	struct stat ret;
	if(lstat(path.c_str(),&ret)==-1) ret.st_dev = 0;
	return ret;
}

static int open(const string &path,int mode=O_RDONLY)
{
	int ret = ::open(path.c_str(),mode);
#ifdef TRACE_H
	plog("open(%s,%d)=%d",path.c_str(),mode,ret);
#endif
	return ret;
}

static string readfile(const string &path)
{
#ifdef TRACE_H
	plog("readfile: %s",path.c_str());
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
	int fd = ::open(path.c_str(),O_RDONLY|O_BINARY);
	string t = read(fd);
	::close(fd);
	return t;
}

static vector<dirent> readdir(const string &path)
{
#ifdef TRACE_H
	plog("readdir: %s",path.c_str());
#endif
	vector<dirent> ret;
	DIR *dir = ::opendir(path.c_str());
	if(!dir) return ret;
	struct dirent e,*ee=0;
	while(::readdir_r(dir,&e,&ee)==0 && ee!=0) ret.push_back(e);
	::closedir(dir);
	return ret;
}

static string finddev(dev_t dev)
{
	for(auto &e:IO_H::readdir("/dev"))
	{
		string ret = string("/dev/") + e.d_name;
		if(IO_H::stat(ret).st_rdev==dev) return ret;
	}
	return "";
}

static bool writefile(const string &path,const string t)
{
#ifdef TRACE_H
	plog("writefile: %s",path.c_str());
#endif
	::remove(path.c_str());
	int fd = ::open(path.c_str(),O_WRONLY|O_CREAT,0755);
	if(fd!=-1)
	{
		size_t n = ::write(fd,t.c_str(),t.size());
		close(fd);
		if(n == t.size()) return true;
	}
	return false;
}

static int send(int fd,const string &a,int flags=MSG_NOSIGNAL)
{
	ssize_t n = ::send(fd,a.c_str(),a.size(),flags);
	//if(n!=a.size()) plog("%ld/%ld",(long)a.size(),(long)n);
	return (size_t)n==a.size()?0:-1;
}

static int gets(int fd,string &ret)
{
	char c;
	ret.resize(0);
	int status;
	while((status=::read(fd,&c,1))!=-1 && status!=0)
	{
		if(c=='\n') break;
		ret.push_back(c);
	}
	if(ret.size() && ret.back()=='\r') ret.resize(ret.size()-1);
	return ret.size() == 0 && (status==-1||status==0) ? -1 : 0;
}

static int gets(std::iostream &fd,string &ret)
{
	int c;
	ret.resize(0);
	while((c=fd.get())!=std::char_traits<char>::eof())
	{
		if(c=='\n') break;
		ret.push_back(c);
	}
	if(ret.size() && ret.back()=='\r') ret.resize(ret.size()-1);
	return ret.size() == 0 && c==std::char_traits<char>::eof() ? -1 : 0;
}

#if 0
int gets(std::iostream &fd,string &ret)
{
	ret.resize(0);
	for(ssize_t x = ret.size();;)
	{
		const int nn=10;
		ret.resize(ret.size() + nn);
		char *t = (char*)ret.c_str() + x;
		fd.getline(t,nn,'\n');
		size_t n = strlen(t);
		if(n && t[n-1]=='\r') --n;
		ret.resize(ret.size() - nn + n);
		if(t[0] && (fd.rdstate()&std::iostream::failbit))
		{
			x += n;
			continue;
		}
		break;
	}
	if(fd.fail()) return -1;
	return 0;
}
#endif

static struct in_addr inaddr(uchar a,uchar b,uchar c,uchar d)
{
	in_addr t;
	t.s_addr = htonl((a<<24)+(b<<16)+(c<<8)+d);
	return t;
}

static unsigned short inport(uchar h,uchar l)
{
	return htons(h*256+l);
}

#if 0
static bool isTcpPort(const char *host, int port)
{
	int so = socket(AF_INET, SOCK_STREAM, 0);
	if (so == -1) return false;
	sockaddr_in a;
	a.sin_family = AF_INET;
	a.sin_addr = resolve(host);
	a.sin_port = htons(port);
	int ret = connect(so, (struct sockaddr*) &a, sizeof(a));
	close(so);
	return ret != -1;
}

static void closeAllSock()
{
	for (int i = 0; i < 100; i++)
	{
		int type = 0;
		socklen_t tlen = sizeof(type);
		if (getsockopt(i, SOL_SOCKET, SO_TYPE, &type, &tlen) == 0) close(i);
	}
}
#endif

struct Host
{
	//string source,protocol,authority,userInfo,user,password,host,port,relative,path,directory,file,query,anchor;
	//foo://username:password@www.example.com:123/hello/world/there.html?name=ferret#foo
	string protocol,host,port,query;
	Host()
	{
	}
	Host(const sockaddr &a):host(STR_H::str(a,'h')),port(STR_H::str(a,'p'))
	{
	}
	static Host parse(const string &x)
	{
		// http://stackoverflow.com/questions/3624651/c-url-parser-using-boost-regex-match
		using namespace std;
		Host ret;
		size_t i=0,t;
		auto b=x.cbegin();
		if(i==x.size()) return ret;
		t = x.find("://",i);
		if(t!=string::npos) ret.protocol=string(b,b+t),i=t+3;
		t = x.find_first_of('/',i);
		if(t==string::npos) t = x.size();
		ret.host=string(b+i,b+t),i=t;
		t = ret.host.find_last_of(':');
		if(t!=string::npos)
		{
			ret.port = string(ret.host,t+1);
			ret.host = string(ret.host,0,t);
		}
		else
			ret.port = "80";
		t = x.find_last_of('/',i);
		if(t!=string::npos) ret.query = string(b+t,x.end());
		return ret;
	}
	string unparse() const
	{
		string ret;
		if(protocol.size()) ret += protocol + "://";
		if(host.size()) ret += host;
		if(port.size()) ret += ":" + port;
		if(query.size()) ret += (query.size() && query[0]=='/' ? "" : "/") + query;
		return ret;
	}
	struct in_addr resolve(const string& host)
	{
		in_addr ret;
		int a,b,c,d;
		if(isdigit(*host.c_str()) && sscanf(host.c_str(),"%d.%d.%d.%d",&a,&b,&c,&d)==4)
		{
			ret.s_addr = IO_H::inaddr(a,b,c,d).s_addr;
		}
		else
		{
			struct hostent *h = gethostbyname(host.c_str());
			if (!h) pthrow("[%s]", host.c_str());
			ret.s_addr = *(u_long*) h->h_addr;
		}
		return ret;
	}
	struct sockaddr addr()
	{
		//if(host.size()==0) pthrow("no host");
		sockaddr_in a;
		a.sin_family = AF_INET;
		a.sin_addr = resolve(host.c_str());
		a.sin_port = htons(atoi(port.c_str()));
		return *(struct sockaddr*)&a;
	}
	int connect()
	{
		struct sockaddr a = addr();
		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(fd==-1) pthrow("socket");
		if(::connect(fd, &a, sizeof(a))==SOCKET_ERROR)
		{
			close(fd);
			return -1;
		}
		return fd;
	}
	int connectT()
	{
		int so = connect();
		if(so==-1) pthrow(unparse());
		return so;
	}
	#ifdef __CURL_CURL_H
	// http://curl.haxx.se/libcurl/c/getinmemory.html
	static size_t getCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realsize = size * nmemb;
		string *ret = (string*)userp;
		auto n = ret->size();
		ret->resize(n+realsize);
		memcpy((char*)ret->c_str()+n,contents,realsize);
		return realsize;
	}
	string get()
	{
		string ret;
		curl_global_init(CURL_GLOBAL_ALL);
		CURL *curl = curl_easy_init();
		if(!curl) return ret;
		curl_easy_setopt(curl, CURLOPT_URL, unparse().c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&ret);
		CURLcode res = curl_easy_perform(curl);
		if(res != CURLE_OK) ret = curl_easy_strerror(res);
		curl_easy_cleanup(curl);
		if(res != CURLE_OK) pthrow(ret);
		curl_global_cleanup();
		return ret;
	}
	#else
	string get()
	{
		string ret;
		int fd = connect();
		//string t = "GET " + query + " HTTP/1.0\r\n\r\n";
		string t = "GET " + query + " HTTP/1.1\r\n"
			+ "User-Agent: net.h/1.0\r\n"
			+ "Host: " + host + "\r\n"
			+ "\r\n";
		return get(t);
		if(::write(fd,t.c_str(),t.size())!=t.size())
		{
			close(fd);
			pthrow("write");
		}
		ret = IO_H::read(fd);
		close(fd);
		if(1==1)
		{
			size_t n = ret.find_first_of("\r\n",0);
			if(n==string::npos) n = ret.size();
			string t(ret,0,n);
			vector<string> tt = ::STR_H::split(t," ",3);
			if(tt.size()<3) pthrow(t);
			if(tt[2]!="OK") pthrow(tt[2]);
		}
		if(1==1)
		{
			//plog("%s",::STR_H::str(ret.c_str(),ret.size()).c_str());
			size_t n = ret.find("\r\n\r\n");
			if(n==string::npos) n = ret.find("\n\n"); else n += 2;
			if(n==string::npos) pthrow(ret); else n += 2;
			ret = string(ret,n);
		}
		return ret;
	}
	#endif
};

class TSocket
{
	// PTHREAD_CANCEL_ASYNCHRONOYS: http://www.linux.org.ru/forum/development/9968671?lastmod=1387971319087
	public:
		typedef std::function<int(int,int,void *param1,void *param2)> ON1;
		typedef std::function<int(int,int,const string&,void *param1,void *param2)> ON2;
		struct ON3 { virtual int operator()(int,int,const string&,void *param1,void *param2) = 0; };
	private:
		struct Entry
		{
			ON1 on1;
			ON2 on2;
			ON3 *on3;
			short events;
			void *param1,*param2;
			int issock;
			Entry(){}
			Entry(int fd,ON1 on,void *param1,void *param2,int events)
			{
				this->on1 = on;
				this->on2 = 0;
				this->on3 = 0;
				this->events = events;
				this->param1 = param1;
				this->param2 = param2;
				this->issock = issocket(fd);
			}
			Entry(int fd,ON2 on,void *param1,void *param2,int events)
			{
				this->on1 = 0;
				this->on2 = on;
				this->on3 = 0;
				this->events = events;
				this->param1 = param1;
				this->param2 = param2;
				this->issock = issocket(fd);
			}
			Entry(int fd,ON3 *on,void *param1,void *param2,int events)
			{
				this->on1 = 0;
				this->on2 = 0;
				this->on3 = on;
				this->events = events;
				this->param1 = param1;
				this->param2 = param2;
				this->issock = issocket(fd);
			}
			friend ostream& operator<<(ostream &ret,const Entry &a)
			{
				return ret << a.issock;
			}
		};
		int wfd;
		map<int,Entry> fds;
		int done;
	public:
		std::mutex mutex;
		std::thread thread;
		static const int POLLERROR = POLLRDHUP|POLLERR|POLLHUP|POLLNVAL;
	public:
		TSocket()
		{
			int pi[2];
			if(::pipe(pi)==-1) pthrow("pipe");
			int rfd = pi[0];
			fcntl(rfd, F_SETFL, fcntl(rfd, F_GETFL) | O_NONBLOCK);
			wfd = pi[1];
			ON2 onpipe = [&done](int fd,int w,const string &a,void *param1,void *param2)
			{
				if(w=='e') return done = 1;
				return 0;
			};
			fds[rfd]=Entry(rfd,onpipe,0,0,POLLIN|POLLPRI|POLLERROR);
			thread = std::thread([&]()
			{
				run();
			});
		}
		int close()
		{
			if(wfd==-1) return -1;
			::close(wfd),wfd=-1;			 // unpoll
			return 0;
		}
		~TSocket()
		{
			close();
			//sched_yield();
																 //thread.detach();
			if(thread.joinable()) thread.join();
		}
		void unpoll()
		{
			if(wfd!=-1) ::write(wfd,"",1);
		}
		template<typename ON> int add(int fd,ON on,void *param1=0,void *param2=0,int events=POLLIN|POLLPRI|POLLERROR)
		{
			int ret = -1;
			mutex.lock();
			if(fds.find(fd)==fds.end())
			{
				fds[fd]=Entry(fd,on,param1,param2,events);
				ret = 0;
			}
			mutex.unlock();
			if(ret==0) unpoll();
			return ret;
		}
		int add1(int fd,ON1 on,void *param1=0,void *param2=0,int events=POLLIN|POLLPRI|POLLERROR){return add(fd,on,param1,param2,events);}
		int add2(int fd,ON2 on,void *param1=0,void *param2=0,int events=POLLIN|POLLPRI|POLLERROR){return add(fd,on,param1,param2,events);}
		int add3(int fd,ON3 *on,void *param1=0,void *param2=0,int events=POLLIN|POLLPRI|POLLERROR){return add(fd,on,param1,param2,events);}
		int remove(int fd)
		{
			int ret = -1;
			mutex.lock();
			if(fds.find(fd)!=fds.end())
			{
				fds.erase(fd);
				ret = 0;
			}
			mutex.unlock();
			if(ret==0) unpoll();
			return ret;
		}
		uint count1(void *p)
		{
			int n = 0;
			for(auto &e:fds)
				if(e.second.param1==p) n++;
			return n;
		}
		uint count2(void *p)
		{
			int n = 0;
			for(auto &e:fds)
				if(e.second.param2==p) n++;
			return n;
		}
		int runonce()
		{
			mutex.lock();
			int i = 0, n = fds.size();
			pollfd pf[n];
			Entry ee[n];
			for(auto &e:fds)
			{
				pf[i].fd = e.first;
				pf[i].events = e.second.events;
				pf[i].revents = 0;
				ee[i] = e.second;
				i++;
			}
			mutex.unlock();
			//plog("poll %d ...",n);
			int x = poll(pf,n,-1);
			if(x==-1||x==0) return 1;
			for(i=0;i<n;i++)
			{
				pollfd &f = pf[i];
				if(f.revents)
				{
					//plog("%d:%s",f.fd,eventstr(f.revents).c_str());
					int w = (f.revents&POLLERROR) ? 'e'
						: (f.revents&POLLOUT) ? 'o'
						: 0;
					string t;
					if((ee[i].on2||ee[i].on3) && (f.revents&POLLIN)) t = ee[i].issock ? IO_H::recv(f.fd) : IO_H::read(f.fd);
					int done = 0;
					try
					{
						if(ee[i].on1) done = ee[i].on1(f.fd,f.events,ee[i].param1,ee[i].param2);
						else if(ee[i].on2) done = ee[i].on2(f.fd,w,t,ee[i].param1,ee[i].param2);
						else if(ee[i].on3) done = (*ee[i].on3)(f.fd,w,t,ee[i].param1,ee[i].param2);
						else done = 1;
					}
					catch(...)
					{
					}
					if(done)							 /// autoclose?
					{
						remove(f.fd);
						//::shutdown(f.fd,SHUT_RDWR);
						::close(f.fd);
					}
				}
			}
			return 0;
		}
		void run()
		{
			done = 0;
			while(!done)
			{
				if(runonce()) break;
			}
			mutex.lock();
			for(auto &e:fds) ::close(e.first);
			fds.erase(fds.begin(),fds.end());
			mutex.unlock();
			plog("TSocket.run done=%d",done);
		}
		static string eventstr(int e)
		{
			vector<const char*> ret;
			if((e&POLLIN)!=0) ret.push_back("IN"),e&=~POLLIN;
			if((e&POLLPRI)!=0) ret.push_back("PRI"),e&=~POLLPRI;
			if((e&POLLOUT)!=0) ret.push_back("OUT"),e&=~POLLOUT;
			if((e&POLLERR)!=0) ret.push_back("ERR"),e&=~POLLERR;
			if((e&POLLRDHUP)!=0) ret.push_back("RDHUP"),e&=~POLLRDHUP;
			if((e&POLLHUP)!=0) ret.push_back("HUP"),e&=~POLLHUP;
			if((e&POLLNVAL)!=0) ret.push_back("NVAL"),e&=~POLLNVAL;
			string t = STR_H::join(ret,"|");
			if(e) t+=STR_H::str(e,"|0x%x");
			return t;
		}
		static int issocket(int fd)
		{
			int type = 0;
			socklen_t tlen = sizeof(type);
			if (getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &tlen) == 0) return 1;
			return 0;
		}
		string str() const
		{
			return STR_H::str(fds);
		}
		friend ostream& operator<<(ostream &ret,const TSocket &a)
		{
			return ret << a.str();
		}
};

NT(IO_H)

#endif
