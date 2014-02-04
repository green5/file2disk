//-h- car.h
#ifndef CAR_H
#define CAR_H
#endif
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/tor.h
#ifndef TOR_H
#define TOR_H tor
#include "std.h"
#include "str.h"
#include "io.h"
extern int debug_;
class Torrent;
class Announce;
struct Torrent
{
	string info_hash;
	string announce;
	struct File
	{
		size_t length;
		string path;
		size_t offset;
		friend ostream& operator<<(ostream& ret,const File &a)
		{
			ret << a.length << ',' << a.offset << "," << a.path;
		}
	};
	vector<File> files;
	vector<string> info_pieces;
	uint pieceLength;
	friend ostream& operator<<(ostream& ret,const Torrent &a)
	{
		ret << "a=" << a.announce << ";";
		ret << "f=" << STR_H::str(a.files) << ";";
		ret << "p=" << a.pieceLength << "/" << a.info_pieces.size() << ";";
		return ret;		
	}
	static void readT(const char *path,Torrent &ret);
	static void readT(const string&,Torrent &ret);
	bool valid() { return info_hash.size() > 0; }
	size_t totalsize()
	{
		return info_pieces.size()*pieceLength;
	}
	File *file(uint fileno)
	{
		if(fileno>=files.size()) return 0;
		return &files[fileno];
	}	
};
struct Announce
{
	std::vector<string> peers;
	int interval;
	string warning;
	friend ostream& operator<<(ostream& ret,const Announce &a)
	{
		for(const auto& p:a.peers) ret << p << ",";
		ret << "i=" << a.interval << ",";
		ret << "w=" << a.warning << ";";
		return ret;		
	}
	static void readT(const Torrent&,Announce &ret);
	static void readT(const string&,Announce &ret);
};
#endif
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/types.h
#ifndef TYPES_H
#define TYPES_H tor
#include "std.h"
#include "io.h"
#include "boost/variant.hpp"
NS(TYPES_H)
struct Msg
{
	static constexpr const char* proto = "BitTorrent protocol";
	static constexpr const char* reserved = "";
	static constexpr const char* id = "-HX0000-21.08.2013";
	template<typename T> struct BaseMsg
	{
		size_t hash() const { return typeid(T).hash_code(); }
		static string demangle(const char *name)
		{
			string ret;
			int status;	
			char *t = abi::__cxa_demangle(name,NULL,NULL,&status); // free() after use or NULL?			 
			if(t==0) return ret;
			char *n = t;
			n = strstr(t,"tor::Msg::"); n = n ? n + 10 : t;
			ret = n;
			free(t);
			///ret = STR_H::replace(ret,"tor::Msg::",""); ?
			return ret;
		}
		virtual string name() const { return demangle(typeid(T).name()); }
		virtual string str() const = 0;
	};
	struct Handshake : BaseMsg<Handshake>
	{
		string pstr; string reserved; string info_hash; string peer_id;
		Handshake(const string &pstr_, const string &reserved_, const string &info_hash_, const string &peer_id_):pstr(pstr_),reserved(reserved_),info_hash(info_hash_),peer_id(peer_id_){}
		string str() const
		{
			string b;
			b += (char)pstr.size();
			b += pstr;
			b += STR_H::resize(reserved,8,0);
			b += STR_H::resize(info_hash,20,0);
			b += STR_H::resize(peer_id,20,0);				
			return b;
		}
		string name() const
		{
			return STD_H::sprintf("Handshake.%s.%s.%d.%s",pstr.c_str(),reserved.c_str(),info_hash.size(),peer_id.c_str());
		}
	};
	struct KeepAlive : BaseMsg<KeepAlive>
	{
		string str() const
		{
			string b;
			b += hton32(0);
			return b;
		}		
	};
	struct Choke : BaseMsg<Choke> 
	{
		string str() const
		{
			string b;
			b += hton32(1);
			b += hton8(0);
			return b;
		}		
	};
	struct Unchoke : BaseMsg<Unchoke>
	{
		string str() const
		{
			string b;
			b += hton32(1);
			b += hton8(1);
			return b;
		}		
	};
	struct Interested : BaseMsg<Interested> 
	{
		string str() const
		{
			string b;
			b += hton32(1);
			b += hton8(2);
			return b;
		}		
	};
	struct NotInterested : BaseMsg<NotInterested> 
	{
		string str() const
		{
			string b;
			b += hton32(1);
			b += hton8(3);
			return b;
		}		
	};
	struct Have : BaseMsg<Have>
	{
		uint32_t piece_index;
		Have(uint32_t piece_index_):piece_index(piece_index_){}		
		string str() const
		{
			string b;
			b += hton32(1+4);
			b += hton8(4);
			b += hton32(piece_index);
			return b;
		}		
		string name() const
		{
			return STD_H::sprintf("Have.%d",(int)piece_index);
		}
	};
	struct Bitfield : BaseMsg<Bitfield>
	{
		string bit;
		Bitfield(const string &bit_):bit(bit_){}
		string str() const
		{
			string b;
			b += hton32(1+bit.size());
			b += hton8(5);
			b += bit;
			return b;
		}		
	};		
	struct Request : BaseMsg<Request>
	{
		uint32_t index;	
		uint32_t begin;
		uint32_t length;
		Request(uint32_t index_,uint32_t begin_,uint32_t length_):index(index_),begin(begin_),length(length_){}
		string str() const
		{
			string b;
			b += hton32(1+12);
			b += hton8(6);
			b += hton32(index);
			b += hton32(begin);
			b += hton32(length);
			return b;			
		}
		string name() const
		{
			return STD_H::sprintf("Request.%d.%d.%d",(int)index,(int)begin,(int)length);
		}
	};
	struct Piece : BaseMsg<Piece>
	{
		uint32_t index;
		uint32_t begin;
		string block;
		Piece(uint32_t index_,uint32_t begin_,const string& block_):index(index_),begin(begin_),block(block_){}
		string str() const
		{
			string b;
			b += hton32(1+8+block.size());
			b += hton8(7);
			b += hton32(index);
			b += hton32(begin);
			b += block;				
			return b;			
		}
		string name() const
		{
			return STD_H::sprintf("Piece.%d.%d.%d",(int)index,(int)begin,(int)block.size());
		}
	};
	struct Cancel : BaseMsg<Cancel>
	{
		uint32_t index;
		uint32_t begin;
		uint32_t length;
		Cancel(uint32_t index_,uint32_t begin_,uint32_t length_):index(index_),begin(begin_),length(length_){}
		string str() const
		{
			string b;
			b += hton32(1+12);
			b += hton8(8);
			b += hton32(index);
			b += hton32(begin);
			b += hton32(length);
			return b;			
		}
	};
	struct Port : BaseMsg<Port>
	{
		uint16_t listen_port;
		Port(uint16_t listen_port_):listen_port(listen_port_){}
		string str() const
		{
			string b;
			b += hton32(3);	
			b += hton8(9);
			b += hton16(listen_port);			
			return b;			
		}
		string name() const
		{
			return STD_H::sprintf("Port.%d",listen_port);
		}
	};
	struct Unknown : BaseMsg<Port>
	{
		uint id;
		Unknown(uint id_):id(id_){}
		string str() const
		{
			return string();
		}
		string name() const
		{
			return STD_H::sprintf("Unknown.%d",id);
		}
	};
	struct Enum
	{
		boost::variant<Handshake,KeepAlive,Choke,Unchoke,Interested,NotInterested,Have,Bitfield,Request,Piece,Cancel,Port,Unknown> value;
		enum Type { handshake,keepalive,choke,unchoke,interested,notinterested,have,bitfield,request,piece,cancel,port,unknown };
		Type type_;		
		template<typename T> Enum(T a):value(a),type_(type())
		{
		}
		struct Name : public boost::static_visitor<string>
		{
			template<typename T> string operator()(const T& t) const { return t.name(); }
		};
		string name() const
		{
			return boost::apply_visitor(Name(),value);
		}
		struct Hash : public boost::static_visitor<size_t>
		{
			template<typename T> size_t operator()(const T& t) const { return t.hash(); }
		};
		size_t hash() const
		{
			return boost::apply_visitor(Hash(),value);
		}
		enum Type type() const
		{
			size_t h = hash();
			if(h==typeid(Handshake).hash_code()) return handshake;
      if(h==typeid(KeepAlive).hash_code()) return keepalive;
      if(h==typeid(Choke).hash_code()) return choke;
      if(h==typeid(Unchoke).hash_code()) return unchoke;
      if(h==typeid(Interested).hash_code()) return interested;
      if(h==typeid(NotInterested).hash_code()) return notinterested;
      if(h==typeid(Have).hash_code()) return have;
      if(h==typeid(Bitfield).hash_code()) return bitfield;
      if(h==typeid(Request).hash_code()) return request;
      if(h==typeid(Piece).hash_code()) return piece;
      if(h==typeid(Cancel).hash_code()) return cancel;
      if(h==typeid(Port).hash_code()) return port;
      if(h==typeid(Unknown).hash_code()) return unknown;
			return unknown;
		}
	};
	template<typename T> static void push(vector<Enum> &ret,const T &a)
	{
		ret.push_back(a);
	}
	static uint unpack(string &ainput,vector<Enum> &ret)
	{
		/// optimize string
		uint done = 0;
		while(true)
		{
			uint n = ainput.size();
			if(n<4) break;
			ulong l = ntoh32(ainput);
			if(l == 0x13426974 && n>=68) // must 13h,BitTorrent protocol
			{
				uint pstrlen = ainput[0];
				push(ret,Msg::Handshake(ainput.substr(1,pstrlen),ainput.substr(1+pstrlen,8),ainput.substr(1+pstrlen+8,20),ainput.substr(1+pstrlen+8+20,20)));
				ainput = STR_H::substring(ainput,1+pstrlen+8+20+20); 
				done += 1+pstrlen+8+20+20;
			}
			else if(n<(l+4)) 
				break;
			else if(l==0)
			{
				push(ret,Msg::KeepAlive());
				ainput = STR_H::substring(ainput,l+4); done += l+4;
			}
			else
			{
				uchar id = ainput[4];
				string msg = ainput.substr(5,l-1); 
				uint m = msg.size();
				ainput = STR_H::substring(ainput,l+4); done += l+4;
				if(id==0) push(ret,Msg::Choke());
				else if(id==1) push(ret,Msg::Unchoke());
				else if(id==2) push(ret,Msg::Interested());
				else if(id==3) push(ret,Msg::NotInterested());
				else if(id==4 && m==4) push(ret,Msg::Have(ntoh32(msg)));
				else if(id==5 && m>=0) push(ret,Msg::Bitfield(msg));
				else if(id==6 && m==12) push(ret,Msg::Request(ntoh32(msg),ntoh32(msg,4),ntoh32(msg,8)));
				else if(id==7 && m>=8) push(ret,Msg::Piece(ntoh32(msg),ntoh32(msg,4),msg.substr(8)));
				else if(id==8 && m==12) push(ret,Msg::Cancel(ntoh32(msg),ntoh32(msg,4),ntoh32(msg,8)));
				else if(id==9 && m==2) push(ret,Msg::Port(ntoh16(msg)));
				else push(ret,Msg::Unknown(id)); //pthrow("id=%d",id);
			}
		}			
		return done;						
	}
	static uint32_t ntoh32(string &s,int n=0)
	{
		uchar *b = (uchar*)s.c_str();
		return (b[n+0]<<24)+(b[n+1]<<16)+(b[n+2]<<8)+b[n+3];
	}
	static uint16_t ntoh16(string &s,int n=0)
	{
		uchar *b = (uchar*)s.c_str();
		return (b[n+0]<<8)+b[n+1];
	}
	static string hton32(uint32_t a)
	{
		string ret;
		ret += (char)((a>>24)&255);
		ret += (char)((a>>16)&255);
		ret += (char)((a>>8)&255);
		ret += (char)(a&255);
		return ret;		
	}
	static string hton16(uint16_t a)
	{
		string ret;
		ret += (char)((a>>8)&255);
		ret += (char)(a&255);
		return ret;		
	}
	static string hton8(uint8_t a)
	{
		string ret;
		ret += (char)(a&255);
		return ret;		
	}
};
struct File;
struct Request;
struct Peer
{
	string host;
	string sock_input;
	Peer(const string &host_):host(host_)
	{
	}
	string str() const
	{
		return host;
	}
	DEFINE_LSTR(Peer)
};
struct File : IO_H::TSocket::ON3
{
	Torrent tor;
	IO_H::TSocket *thr; /// one thread per file
	std::vector<TOR_H::Peer> peers_;
	File():thr(0)
	{
	}
	File(const char *path):thr(0)
	{
		openT(path);
	}
	~File()
	{
		stop();
	}
	bool valid()
	{
		return tor.valid() && thr != 0 && peers_.size();
	}
	void addPeer(const TOR_H::Peer &a)
	{
		peers_.push_back(a);
	}
	void setPeer(const std::vector<TOR_H::Peer> &a)
	{	
		peers_ = a;
	}
	void openT(const char *path)
	{
		Torrent::readT(path,tor);
	}
	bool start()
	{
		if(!tor.valid()) return false;
		if(thr==0) thr = new IO_H::TSocket();
	}
	void stop()
	{
		if(thr) delete thr;
		thr = 0;
	}
	size_t fsize(int fileno)
	{
		if(!tor.valid()) return 0;
		auto f = tor.file(fileno);
		return f->length;		
	}
	size_t bestsize(int fileno,size_t count,size_t off,int how);
	int read(int fileno,void *buf,size_t count,size_t off,size_t *readbytes=0);
	int operator()(int fd,int why,const string &a,void *param1,void *param2);
};
NT(TOR_H)
#endif
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/main.cpp
#include "main.h"
#ifndef CAR_H
#include "tor.h"
#include "types.h"
#endif
int torsrc_register();
int torsrc_init(int *ac,char ***av);
int torsrc_play();
NS(TOR_H)
NT(TOR_H)
#ifdef TOR_MAIN
string peers_ = STD_H::opt::define("-p",peers_,"");
int n_ = STD_H::opt::define("-n",n_,16);
int debug_ = STD_H::opt::define("-debug",debug_,0);
#include <fstream>
void readfile(const string &file)
{
	std::ifstream is(file);
	std::stringbuf b;
	is >> &b;
	string input = b.str();
	vector<TYPES_H::Msg::Enum> ii;
	TYPES_H::Msg::unpack(input,ii);
	for(auto &msg:ii)
	{
		plog("%s",msg.name().c_str());
	}
	if(input.size()) plog("input=%d",(int)input.size());	
}
void test(int n)
{
	if(n==1)
	{
		//IO_H::Host h = IO_H::Host::parse("foo://username:password@www.example.com:123/hello/world/there.html?name=ferret#foo");
		IO_H::Host h = IO_H::Host::parse("http://u3:80/index.php");
		cout << "host=" << h.unparse() << "\n";
		cout << h.get();
	}
	if(n==2)
	{
		TOR_H::File h("q.torrent");
		h.start();
		h.stop();
		cout << "T:" << h.tor << "\n";		
		Announce a;
		Announce::readT(h.tor,a);		
		cout << "A:" << a << "\n";
	}
	if(n==3)
	{
		TOR_H::File h("q.torrent");
		cout << PLINE << h.tor << "\n";
		Announce a;
		if(1==0)
		{
			Announce::readT(h.tor,a);		
			cout << PLINE << a << "\n";
			if(a.peers.size()==0) plog("no peers");		
		}
		std::vector<TOR_H::Peer> pp;
		if(1==1)
		{
			if(!peers_.size()) peers_ = "192.168.8.3:6881,192.168.8.6:6881,192.168.8.5:6881";
			for(auto &a:STR_H::split(peers_,",")) pp.push_back(TOR_H::Peer(a));				
			if(pp.size()==0) pexit("no peers");
		}
		h.setPeer(pp);
		int no = 0;
		size_t fsize = h.fsize(no);
		size_t block = 10*1000000;
		//if(no==0) block = n_*5*262144;
		char *buf = (char*)calloc(1,block);
		if(buf==0) exit(1);
		std::ofstream os("/tmp/q.tor");
		h.start();
		double t1 = STD_H::ns();
		int error = 0;
		plog("connecting to %s",STR_H::str(pp).c_str());
		for(size_t off=0;off<fsize;)
		{
			size_t n = STD_H::min2(block,fsize-off);
			error = h.read(no,buf,n,off);
			if(error) break;
			os.write(buf,n);			
			off += n;
		}
		t1 = STD_H::ns() - t1;
		if(error)
			plog("error=%d",error); 
		else
			plog("%ekb/s",1e-3*fsize/t1);
		n = STD_H::nthread(); plog("threads=%d",n);
		h.stop(); sleep(1);
		n = STD_H::nthread(); plog("threads=%d",n);
	}
	if(n==4)
	{
		if(torsrc_init(0,0)<0) pexit("init");
		if(torsrc_register()<0) pexit("register");
		if(torsrc_play()<0) pexit("play");
	}
}
int main(int ac,char *av[])
{
	string rfile_ = STD_H::opt::define("-r",rfile_,"");
	STD_H::opt::main(ac,av);
	if(rfile_.size()) readfile(rfile_),exit(0);
	for(int i=1;i<ac;i++) test(atoi(av[i]));
  return 0;
}
#endif
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/tor.cpp
#include "std.h"
#include "str.h"
#include "io.h"
#ifndef CAR_H
#include "tor.h"
#include "types.h"
#endif
#include "lib/openbsd-compat/sha1.h"
#include "lib/openbsd-compat/sha1.c"
struct Value
{
	typedef std::vector<Value> List;
	typedef std::map<string,Value> Dict;
	char t;
	int i;
	string s;
	List l;
	Dict d;
	Value():t(0){}
	Value(const int &o):t('i'),i(o){}
	Value(const string&o):t('s'),s(o){}
	Value(const List&o):t('l'),l(o){}
	Value(const Dict&o):t('d'),d(o){}
	int type() { return t; }
	int intT() { passert(type()=='i'); return i;}
	int intT(int z) { if(type()=='i'); return i; return z; }
	string &stringT() {	passert(type()=='s',"%c",type()); return s; }
	string &stringT(string &z) {	if(type()=='s'); return s; return z; } 
	List &listT() {	passert(type()=='l');	return l; }
	Dict &dictT() {	passert(type()=='d');	return d;	}
	friend ostream& operator<<(ostream &ret,const Value &v)
	{
		if(v.t=='s') ret << v.s.size() << ':' << v.s;
		else if(v.t=='i') ret << 'i' << v.i << 'e'; 			
		else if(v.t=='l')
		{
			ret << 'l';
			for(auto &l:v.l) ret << l;
			ret << 'e';
		}
		else if(v.t=='d')
		{
			ret << 'd';
			for(auto &i:v.d) ret << i.first.size() << ':' << i.first << i.second;
			ret << 'e';
		}
		else pthrow("bad type %c",v.t);
		return ret;
	}
};
using std::istream;
namespace TOR_H
{
	Value readInt(istream &is,int n,char eoi)
	{
		int sign = 0;
		for(;is.good();)
		{
			int c = is.get();
			if(c==EOF) break;
			if(c=='+' && sign==0) sign=1;
			else if(c=='-' && sign==0) sign=-1;
			else if(c>='0' && c <= '9') n = n*10 + c - '0';
			else if(c==eoi) 
			{
				n *= sign?sign:1;
				return n;
			}
			else break;
		}
		pthrow("Eof");
	}
	Value readStr(istream &is,int b)
	{
		Value n = readInt(is,b,':');
		string ret(n.i,0);
		is.read((char*)ret.c_str(),n.i);
		if(!is.good()) pthrow("Eof");
		return ret;
	}
	Value read(istream &is);
	Value readList(istream &is)
	{
		Value::List ret;
		while(true)
		{
			Value x = read(is);
			if(x.t==0) return ret;
			ret.push_back(x);
		}
		pthrow("Err");
	}
	Value readDict(istream &is)
	{
		Value::Dict ret;
		while(1)
		{
			Value k = read(is);
			if(k.t==0) break;
			if(k.t!='s') pthrow("Err");
			ret[k.s] = read(is);
		}
		return ret;
	}
	Value read(istream &is)
	{
		Value ret;
		int c = is.get();
		if(!is.good()||c==EOF) pthrow("Err");
		if(1==0) {}
		else if(c=='i') ret = readInt(is,0,'e');
		else if(c>='0' && c<='9') ret = readStr(is,c-'0');
		else if(c=='l') ret = readList(is);
		else if(c=='d') ret = readDict(is);
		else if(c=='e') ret = Value();
		else pthrow("Err c=%c",c);
		return ret;
	}
}
void Torrent::readT(const char *path,Torrent &ret)
{
	std::ifstream is(path);
	std::stringbuf b;
	is >> &b;
	is.close();
	readT(b.str(),ret);
}
void Torrent::readT(const string &a,Torrent &ret)
{
	std::istringstream b(a);
	Value v = tor::read(b);
	passert(v.type()=='d');
	passert(v.d["announce"].type()=='s');
	ret.announce = v.d["announce"].s;
	passert(v.d["info"].type()=='d');
	if(1==1)
	{
		std::ostringstream info;
		info << v.d["info"];
		const string t(info.str());
		if(1==1)
		{
			SHA1_CTX sha;
			u_int8_t result[SHA1_DIGEST_LENGTH];
		  SHA1Init(&sha);
		  SHA1Update(&sha,(u_int8_t*)t.c_str(),t.size());
		  SHA1Final(result, &sha);
			ret.info_hash = string((char*)result,SHA1_DIGEST_LENGTH);
			//plog("hash=%s",STR_H::str(ret.info_hash.c_str(),ret.info_hash.size()).c_str());
		}
	}
	Value &info = v.d["info"];
	if(1==1)
	{
		if(info.d["files"].type()=='l')
		{
			size_t offset = 0;
			for(auto &i:info.d["files"].l)
			{
				passert(i.type()=='d');
				Torrent::File x;
				x.length = i.d["length"].intT(); 				
				x.offset = offset;
				offset += x.length;
				for(Value &p:i.d["path"].listT()) x.path.append((x.path.size()?"/":"")+p.stringT());
				ret.files.push_back(x);
			}
		}
		else
		{
			Torrent::File x;
			x.length = info.d["length"].intT();
			x.offset = 0;
			x.path = string();
			ret.files.push_back(x);
		}
	}
	if(1==1)
	{
		string &s = info.d["pieces"].stringT();
		uint n = s.size();
		if((n%20)!=0) pthrow("pieces = %d",n);
		for(uint i=0;i<n/20;i++) ret.info_pieces.push_back(s.substr(i*20,20));
	}				
	ret.pieceLength = info.d["piece length"].intT();
}
void Announce::readT(const string& a,Announce &ret)
{
	std::istringstream b(a);
	Value v = tor::read(b); 
	// d={interval:=i=3394,min interval:=i=3394,peers:=s=[6][.s...,warning message:=s=[22]Torrent not registered,}
	passert(v.type()=='d');
	passert(v.d["peers"].type()=='s');
	const string &peers = v.d["peers"].s;
	if((peers.size()%6)==0) // binary model
	{
		for(int i=0;i<peers.size();i+=6)
		{
			const char *p = peers.c_str()+i;
			sockaddr_in a;
			a.sin_family = AF_INET;
			a.sin_addr = IO_H::inaddr(p[0],p[1],p[2],p[3]);
			a.sin_port = IO_H::inport(p[4],p[5]);
			ret.peers.push_back(IO_H::Host(*(struct sockaddr*)&a).unparse());
		}
	}
	ret.interval = v.d["interval"].intT(0); // seconds		
	ret.warning = v.d["warning message"].stringT(ret.warning);
}
//#include <boost/network/protocol/http/client.hpp>
//namespace http = boost::network::http;namespace net  = boost::network;
void Announce::readT(const Torrent& t,Announce &ret)
{
	string id("-HX0000-21.08.2013xx");
	string port("6889");
	string u = t.announce;
  passert(u.find("?")!=string::npos);
	u.append("&info_hash="+STR_H::urlencode(t.info_hash));
	u.append("&peer_id="+STR_H::urlencode(STR_H::resize(id,20,' ')));
	u.append("&port="+port); // peer port?
	u.append("&uploaded=0&downloaded=0&left=0&event=started");
#ifdef BOOST_NETLIB_VERSION
	http::client client;
	http::client::request request(u);
	request << net::header("User-Agent","tor/1.0");
	http::client::response response = client.get(request);
	string b = body(response);
	passert(b.size() && b[0]=='d',b);
	return read(b,ret);
#endif
	IO_H::Host h = IO_H::Host::parse(u);
	readT(h.get(),ret);
}
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/types.cpp
#include "std.h"
#include "str.h"
#include "io.h"
#ifndef CAR_H
#include "tor.h"
#include "types.h"
#endif
#include <thread>
#include <mutex>
#include <condition_variable> 
namespace TOR_H
{
	struct Block
	{
		std::mutex mtx;
		std::condition_variable cv;
		int status;
		Block():status(0){}
		int wait()
		{
			std::unique_lock<std::mutex> lck(mtx);
		  while(!status) cv.wait(lck);	
			return status;
		}
		void signal(int w)		
		{
		  std::unique_lock<std::mutex> lck(mtx);
		  status = w;
		  cv.notify_one();				
		}
	};
	struct Request
	{
		struct Req
		{
			public:
			size_t index,off,size; 
			void *ptr; 
			Req(size_t index_,size_t off_,size_t size_,void *ptr_):index(index_),off(off_),size(size_),ptr(ptr_)
				,status_(0),peer_(0)
			{}
			string str() const
			{
				return STD_H::sprintf("%d.%d.%d=%d",(int)index,(int)off,(int)size,status_);
			}
			private:
			bool status_;
			Peer *peer_; 
			public:
			int done() const
			{
				return status_==1;
			}
			Peer *peer() const
			{
				return peer_;
			}				
			void setstatus(int done)
			{
				status_ = done;
			}
			void setpeer(Peer *peer)
			{
				peer_ = peer;
			}
		};
		std::vector<Req> sub;
		Block wait;
		Request(void *buf,size_t offset1,size_t offset2,size_t pieceLength,size_t bs)
		{
			char *ptr = (char*)buf;
			bs = STD_H::min2(bs,pieceLength);
			for(size_t b=offset1;b<offset2;)
			{
				size_t e = b + bs;
				size_t pend = b + pieceLength - b%pieceLength;
				if(e>pend) e = pend;				
				if(e>offset2) e = offset2;
				size_t size = e - b;
				sub.push_back(Req(b/pieceLength,b%pieceLength,size,ptr));
				b += size;
				ptr += size;
			}
		}
		void free()
		{
			for(Req &i:sub) i.setpeer(0);
		}
		size_t alloc(Peer *peer,size_t size)
		{
			size_t n = 0;
			for(Req &i:sub)
			{
				if(n>size) break;
				if(!i.done() && i.peer()==0) i.setpeer(peer), n+=i.size;
			}
			return n;
		}
		int recv(size_t index,size_t begin,const string &block)
		{
			Req *find = 0;			
			for(Req &i:sub)
			{
				if(i.index==index && i.off==begin && i.size==block.size())
				{
					find = &i;
					break;
				}
				if(i.index>index && i.off>begin) break; /// map<index>
			}
			if(!find) return 0; ///
			memcpy(find->ptr,block.c_str(),find->size);
			find->setstatus(1);
			return 1;
		}
		void send(Peer *peer,int fd)
		{
			string t;
			for(auto &i:sub) 
			{
				if(!i.done() && i.peer()==peer) 
				{
					//plog("Request %ld,%ld,%ld",(long)i.index,(long)i.off,(long)i.size);
					t += Msg::Request(i.index,i.off,i.size).str();
				}
			}
			if(IO_H::send(fd,t)==-1) PLOG;
		}
		int isdone(Peer *peer=0)
		{
			for(auto &i:sub)
			{
				if(!i.done() && (peer==0 || peer==i.peer()) ) return 0;
			}			
			return 1;
		}
		size_t donesize(Peer *peer=0)
		{
			size_t n = 0;
			for(auto &i:sub)
			{
				if(i.done() && (peer==0 || peer==i.peer()) ) n += i.size;
			}
			return n;
		}
		size_t continuous() 
		{
			size_t n = 0;
			for(auto &i:sub)
			{
				if(i.done()) n += i.size; else break;
			}
			return n;
		}
		size_t undonesize(Peer *peer=0)
		{
			size_t n = 0;
			for(auto &i:sub)
			{
				if(!i.done() && (peer==0 || peer==i.peer()) ) n += i.size;
			}
			return n;
		}
		string str() const
		{
			string ret;
			size_t index = sub.size() ? sub[0].index : 0;
			for(auto &i:sub)
			{
				if(index!=i.index) ret += '_',index=i.index;
				if(i.done()) ret += '1';
				else if(i.peer()) ret += 'X';
				else ret += '0';
			}			
			return ret;
		}
	};	
	size_t File::bestsize(int fileno,size_t count,size_t off,int how)
	{
		Torrent::File *f = tor.file(fileno);
		if(f==0) return count;
		size_t b = f->offset + off;
		size_t e = f->offset + off + (how==2?3*1024*1024:count); 
		size_t eof = f->offset + f->length; 
		size_t tof = tor.totalsize();
		if(e>eof) e = eof;
		if(e>tof) e = tof;
		size_t pieceLength = tor.pieceLength;
		e = how == 0 ? pieceLength*(e/pieceLength)
			: pieceLength*((e+pieceLength-1)/pieceLength);
		if(b>=e) return count;
		return e - b;
	}
	template<typename T> static void send(int fd,const T& a)
	{
		//plog(a.name());
		if(IO_H::send(fd,a.str())==-1) perr(a.name());
	}
	int File::operator()(int so,int why,const string &a,void *param1,void *param2)
	{
		Peer *peer = (Peer*)param1;
		Request *current = (Request*)param2;
		if(a.size()) peer->sock_input += a;
		if(peer->sock_input.size())
		{
			vector<Msg::Enum> ii;
			Msg::unpack(peer->sock_input,ii);
			for(auto &msg:ii)
			{
				switch(msg.type_)
				{
					case Msg::Enum::keepalive: 
						send(so,Msg::KeepAlive());
						break;
					case Msg::Enum::handshake: 
					{
						Msg::Handshake &h = boost::get<Msg::Handshake>(msg.value);
						break;	
					}
					case Msg::Enum::unchoke:
						current->send(peer,so);
						break;
					case Msg::Enum::piece:
					{
						const Msg::Piece &h = boost::get<Msg::Piece>(msg.value); // index,begin,block
						current->recv(h.index,h.begin,h.block);
						///plog("%d:%s %ld/%ld",so,msg.name().c_str(),(long)current->donesize(peer),(long)current->undonesize(peer));
						//plog(current->str());
						if(current->isdone(peer)) why='d';
						break;						
					}				
					case Msg::Enum::choke:
					case Msg::Enum::interested:
					case Msg::Enum::notinterested:
					case Msg::Enum::request:
					case Msg::Enum::cancel:
					case Msg::Enum::port:
					default: 
						plog("%d:%s",so,msg.name().c_str());
						break;
					case Msg::Enum::bitfield:
					case Msg::Enum::have:
						break;
				}
			}
		}				
		if(why=='e'||why=='d') 
		{
			if(thr->count2(current)==1) current->wait.signal(why); 
			return 1;
		}
		return 0;
	}
	int File::read(int fileno,void *buf,size_t count,size_t off,size_t *readbytes)
	{
		if(readbytes) *readbytes = 0;
		if(!valid()) return __LINE__;
		size_t pieceLength = tor.pieceLength;
		Torrent::File *f = tor.file(fileno);
		if(f==0) return __LINE__;
		size_t beg = f->offset + off;
		size_t end = f->offset + off + count;
		size_t eof = f->offset + f->length;
		size_t tof = tor.totalsize();
		if(end>eof) end = eof;
		if(end>tof) end = tof;
		size_t size = end - beg;
		//plog("read(%d,[%ld/%ld],%ld) b=%ld/%ld e=%ld/%ld f=%ld/%ld t=%ld/%ld",fileno,(long)count,(long)size,(long)off,(long)(beg/pieceLength),(long)(beg%pieceLength),(long)(end/pieceLength),(long)(end%pieceLength),(long)(eof/pieceLength),(long)(eof%pieceLength),(long)(tof/pieceLength),(long)(tof%pieceLength));
		if(!size)
		{
			return -1; // EOF
		}
		Request req(buf,beg,end,pieceLength,16*1024);
		for(int i=0;!req.isdone();i++)
		{
			if(i==3)
			{
				size_t n = req.continuous();
				if(readbytes) *readbytes = n; 
			 	return n ? 0 : __LINE__;
			}
			req.free();
			for(auto &peer:peers_)
				req.alloc(&peer,3*1024*1024);
			size_t n1 = req.donesize();
			req.wait.status = 0;
			for(auto &peer:peers_)
			{
				if(req.isdone(&peer)) continue;
				//plog("connect %s %ld",peer.host.c_str(),req.undonesize(&peer));
				int so = IO_H::Host::parse(peer.host).connect();
				if(so==-1) continue;
				peer.sock_input.resize(0);
				thr->add3(so,this,&peer,&req);
				send(so,Msg::Handshake(Msg::proto,Msg::reserved,tor.info_hash,Msg::id));
				send(so,Msg::Interested()); 
			}
			int why = req.wait.wait(); // peer done or error
			size_t n2 = req.donesize();
			//plog("read(%d,%ld,%ld):size=%ld/%ld why=%c",fileno,(long)count,(long)off,(long)req.donesize(),(long)req.undonesize(),why);
			if(n2>n1) i=-1;		
		}
		if(readbytes) *readbytes = size;
		return 0;
	}
}
//-h- /home/green/src/gstreamer/1gst/torsrc.cpp/torsrc.c
#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#ifndef TOR_H
#include "tor.h"
#include "types.h"
#endif
struct TorFile
{
	TOR_H::File f_;
	std::vector<TOR_H::Peer> peers;
	uint fileno;
	string path;
	guint64 offset;
	TorFile():fileno(0)
	{
		peers = {TOR_H::Peer("192.168.8.3:6881")};
	}
	gboolean start()
	{
		try
		{
			offset = 0;
			f_.openT(path.c_str());
			f_.setPeer(peers);
			f_.start();
		}
		catch(...)
		{
			perr("Exception"); return false;
		}
		return true;
	}
	gboolean stop()
	{
		f_.stop();
		return true;
	}
	gboolean get_size(guint64 *size)
	{
		guint64 n = f_.fsize(fileno);
		if(!n) return false;
		*size = n;
		return true;
	}
	gboolean seek(GstSegment* segment)
	{
		offset = segment->start;
		//plog("seek(%ld)",(long)offset);
		return true;
	}
	GstFlowReturn read(size_t blocksize, GstBuffer** outbuf)
	{
		size_t count = f_.bestsize(fileno,blocksize,offset,2);
		guint64 n = offset;
		///if(offset) offset += 100000;
		GstFlowReturn ret = read(count, offset, outbuf);
		return ret;
	}
	GstFlowReturn read(size_t blocksize, guint64 &offset, GstBuffer** outbuf)
	{
		GstBuffer *buf = gst_buffer_new_allocate(NULL, blocksize, NULL);
		if(buf == NULL)
		{
			perr("alloc_failed");
			return GST_FLOW_ERROR;
		}
		GstMapInfo info;
		if(!gst_buffer_map(buf, &info, GST_MAP_WRITE))
		{
			perr("map_failed");
			gst_buffer_unref (buf);
			return GST_FLOW_ERROR;
		}
		size_t readbytes = 0;
		//plog("read(%d,[%ld],%ld-%ld",fileno,(long)blocksize,(long)offset,(long)offset+blocksize);
		int error = f_.read(fileno,info.data,blocksize,offset,&readbytes);
		//plog("read=%d %ld-%ld [%ld]",error,(long)offset,(long)offset+readbytes,(long)readbytes);
		if(error == -1) //readbytes == 0
		{
			gst_buffer_unmap (buf, &info);
			gst_buffer_unref (buf);
			return GST_FLOW_EOS;
		}
		if(error)
		{
			perr("read_error(%d,%ld,%ld)=%d",fileno,(long)blocksize,(long)offset,error);
			gst_buffer_unmap (buf, &info);
			gst_buffer_unref (buf);
			return GST_FLOW_ERROR;
		}
		gst_buffer_unmap (buf, &info);
		gst_buffer_resize (buf, 0, readbytes);
		GST_BUFFER_OFFSET (buf) = offset;
		GST_BUFFER_TIMESTAMP (buf) = GST_CLOCK_TIME_NONE;
		offset += readbytes;
		*outbuf = buf;
		return GST_FLOW_OK;
	}
};
G_BEGIN_DECLS
#define GST_TYPE_TORSRC   (gst_torsrc_get_type())
#define GST_TORSRC(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_TORSRC,GstTorSrc))
#define GST_TORSRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_TORSRC,GstTorSrcClass))
#define GST_IS_TORSRC(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_TORSRC))
#define GST_IS_TORSRC_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_TORSRC))
typedef struct _GstTorSrc GstTorSrc;
typedef struct _GstTorSrcClass GstTorSrcClass;
struct _GstTorSrc
{
	GstPushSrc base_torsrc;
	GstPad *srcpad;
	TorFile *fd_;
  GMutex qlock;                /* lock for queue (vs object lock) */
};
struct _GstTorSrcClass
{
	GstPushSrcClass base_torsrc_class;
};
GType gst_torsrc_get_type (void);
G_END_DECLS
GST_DEBUG_CATEGORY_STATIC (gst_torsrc_debug_category);
#define GST_CAT_DEFAULT gst_torsrc_debug_category
#define GST_TORSRC_MUTEX_LOCK(q) G_STMT_START {                          \
  g_mutex_lock (&q->qlock);                                              \
} G_STMT_END
#define GST_TORSRC_MUTEX_LOCK_CHECK(q,res,label) G_STMT_START {         \
  GST_TORSRC_MUTEX_LOCK (q);                                            \
  if (res != GST_FLOW_OK)                                               \
    goto label;                                                         \
} G_STMT_END
#define GST_TORSRC_MUTEX_UNLOCK(q) G_STMT_START {                        \
  g_mutex_unlock (&q->qlock);                                            \
} G_STMT_END
/* prototypes */
static void gst_torsrc_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_torsrc_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec);
static void gst_torsrc_dispose (GObject * object);
static void gst_torsrc_finalize (GObject * object);
static gboolean gst_torsrc_negotiate (GstBaseSrc * src);
static gboolean gst_torsrc_start (GstBaseSrc * src);
static gboolean gst_torsrc_stop (GstBaseSrc * src);
static gboolean gst_torsrc_get_size (GstBaseSrc * src, guint64 * size);
static gboolean gst_torsrc_is_seekable (GstBaseSrc * src);
static gboolean gst_torsrc_do_seek (GstBaseSrc * src, GstSegment * segment);
static GstFlowReturn gst_torsrc_create (GstPushSrc * src, GstBuffer ** buf);
/* pad templates */
static GstStaticPadTemplate gst_torsrc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
GST_PAD_SRC,
GST_PAD_ALWAYS,
GST_STATIC_CAPS ("application/unknown")
);
/* class initialization */
G_DEFINE_TYPE_WITH_CODE (GstTorSrc, gst_torsrc, GST_TYPE_PUSH_SRC,
GST_DEBUG_CATEGORY_INIT (gst_torsrc_debug_category, "torsrc", 0, "debug category for torsrc element"));
enum
{
	PROP_0,
	PROP_PATH,
	PROP_FILENO,
	PROP_LAST,
};
static void
gst_torsrc_class_init (GstTorSrcClass * klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);
	GstPushSrcClass *push_src_class = GST_PUSH_SRC_CLASS (klass);
	/* Setting up pads and setting metadata should be moved to
		 base_class_init if you intend to subclass this class. */
	gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
		gst_static_pad_template_get (&gst_torsrc_src_template));
	gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
		"FIXME Long name", "Generic", "FIXME Description",
		"FIXME <fixme@example.com>");
	gobject_class->set_property = gst_torsrc_set_property;
	gobject_class->get_property = gst_torsrc_get_property;
	g_object_class_install_property (gobject_class, PROP_PATH,
		g_param_spec_string ("path", "Torrent File Location", "Location of torrent file",
		NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
	g_object_class_install_property (gobject_class, PROP_FILENO,
		g_param_spec_uint ("fileno", "Torrent Files number", "Id of torrent subfile",
		0, G_MAXUINT, 0, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
	gobject_class->dispose = gst_torsrc_dispose;
	gobject_class->finalize = gst_torsrc_finalize;
	base_src_class->negotiate = GST_DEBUG_FUNCPTR (gst_torsrc_negotiate);
	base_src_class->start = GST_DEBUG_FUNCPTR (gst_torsrc_start);
	base_src_class->stop = GST_DEBUG_FUNCPTR (gst_torsrc_stop);
	base_src_class->get_size = GST_DEBUG_FUNCPTR (gst_torsrc_get_size);
	base_src_class->is_seekable = GST_DEBUG_FUNCPTR (gst_torsrc_is_seekable);
	base_src_class->do_seek = GST_DEBUG_FUNCPTR (gst_torsrc_do_seek);
	push_src_class->create = GST_DEBUG_FUNCPTR (gst_torsrc_create);
}
static void gst_torsrc_init (GstTorSrc *torsrc)
{
	torsrc->srcpad = gst_pad_new_from_static_template (&gst_torsrc_src_template,"src");
	torsrc->fd_ = new TorFile;
}
void gst_torsrc_finalize (GObject * object)
{
	GstTorSrc *torsrc = GST_TORSRC (object);
	/* clean up object here */
	delete torsrc->fd_;	
	G_OBJECT_CLASS (gst_torsrc_parent_class)->finalize (object);
}
void gst_torsrc_dispose (GObject * object)
{
	/* GstTorSrc *torsrc = GST_TORSRC (object); */
	/* clean up as possible.  may be called multiple times */
	G_OBJECT_CLASS (gst_torsrc_parent_class)->dispose (object);
}
void gst_torsrc_set_property (GObject * object, guint property_id,
const GValue * value, GParamSpec * pspec)
{
	GstTorSrc *torsrc = GST_TORSRC (object);
  GST_TORSRC_MUTEX_LOCK (torsrc);
	switch (property_id)
	{
		case PROP_PATH:
			torsrc->fd_->path = g_value_get_string (value);
			break;
		case PROP_FILENO:
			torsrc->fd_->fileno = g_value_get_uint (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
  GST_TORSRC_MUTEX_UNLOCK (torsrc);
}
void
gst_torsrc_get_property (GObject * object, guint property_id,
GValue * value, GParamSpec * pspec)
{
	GstTorSrc *torsrc = GST_TORSRC (object); 
  GST_TORSRC_MUTEX_LOCK (torsrc);
	switch (property_id)
	{
		case PROP_PATH:
      g_value_set_string (value, torsrc->fd_->path.c_str());
			break;
		case PROP_FILENO:
      g_value_set_uint (value, torsrc->fd_->fileno);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
  GST_TORSRC_MUTEX_UNLOCK (torsrc);
}
static gboolean gst_torsrc_negotiate (GstBaseSrc * src)
{
	return TRUE;
}
static gboolean gst_torsrc_start (GstBaseSrc * src)
{
	GstTorSrc *torsrc = GST_TORSRC (src);
	return torsrc->fd_->start();
}
static gboolean gst_torsrc_stop (GstBaseSrc * src)
{
	GstTorSrc *torsrc = GST_TORSRC (src);
	return torsrc->fd_->stop();
}
static gboolean gst_torsrc_get_size (GstBaseSrc * src, guint64 * size)
{
	GstTorSrc *torsrc = GST_TORSRC (src);
	return torsrc->fd_->get_size(size);
}
static gboolean gst_torsrc_is_seekable (GstBaseSrc * src)
{
	return TRUE;
}
static gboolean gst_torsrc_do_seek (GstBaseSrc * src, GstSegment * segment)
{
	GstTorSrc *torsrc = GST_TORSRC (src);
	return torsrc->fd_->seek(segment);
}
static GstFlowReturn gst_torsrc_create (GstPushSrc * src, GstBuffer ** buf)
{
	GstTorSrc *torsrc = GST_TORSRC (src);
	size_t blocksize = GST_BASE_SRC(src)->blocksize;
	GstFlowReturn ret = torsrc->fd_->read(blocksize, buf);
	return ret;
}
static gboolean plugin_init (GstPlugin * plugin)
{
	return gst_element_register (plugin, "torsrc", GST_RANK_NONE, GST_TYPE_TORSRC);
}
static gboolean gst_bus_async_callback(GstBus *bus, GstMessage *msg, void *user_data)
{
	GMainLoop *loop = (GMainLoop*)user_data;
	switch(GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_EOS:
			plog("End-of-stream=%d",GST_MESSAGE_EOS);
			if(loop) g_main_loop_quit(loop);
			break;
		case GST_MESSAGE_ERROR:
		{
			GError *err;
			gst_message_parse_error(msg, &err, NULL);
			plog("%s", err->message);
			g_error_free(err);
			if(loop) g_main_loop_quit(loop);
			break;
		}
	}
	return true;
}
int torsrc_play()
{
	GMainLoop *loop;
	GstBus *bus;
	GError* error=0;
	GstElement *bin_;
	if(1==1)
	{
		bin_ = gst_parse_launch("torsrc path=q.torrent fileno=0 ! decodebin name=a a.! autoaudiosink a.! autovideosink",&error);
		if(error) plog("%s",error->message),g_error_free(error);
		if(bin_==0||error) return -1;
		gst_element_set_state(bin_,GST_STATE_PAUSED);
	}
	if(1==1)
	{
		GstElement *pipeline = bin_;
		loop = g_main_loop_new(NULL, FALSE);
		bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
		gst_bus_add_watch(bus,gst_bus_async_callback,loop); 
		gst_object_unref(bus);                                	
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
		g_main_loop_run(loop);
		gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
	}
	gst_object_unref(bin_);
}
int torsrc_register()
{
	GError* error=0;
	gst_init_check(0,NULL,&error);
	if(error) plog("%s",error->message),g_error_free(error),error=0;
	if(!gst_plugin_register_static(
		GST_VERSION_MAJOR,GST_VERSION_MINOR,"torsrc","TorSrc"
		,plugin_init
		,"0.1","LGPL","source","PACKAGE_NAME","GST_PACKAGE_ORIGIN")) return -1;
	return 0;
}
int torsrc_init(int *ac,char ***av)
{
	GError* error=0;
	gst_init_check(ac,av,&error);
	if(error) perr("%s",error->message),g_error_free(error);
	return error ? -1 : 0;
}
