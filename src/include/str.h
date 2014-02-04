#ifndef STR_H
#define STR_H str

#include "std.h"
#include <vector>

#ifndef __STRING
#define __STRING(x) #x
#endif
#define CONCATS_(x,y) __STRING(x) y

#ifndef NL
#define NL std::endl
#endif

#define DEFINE_LSTR(T) friend ostream& operator<<(ostream &ret,const T &a) {return ret<<a.str();}

namespace STR_H {

int gcd(const ::std::vector<int> &a); // math.hpp

#if 0
template<typename T>
...
vsplit(const T* first,const T* last,const std::vector<> &delim,unsigned int limit=0)
{
	typedef std::pair<const T*,const T*> ret_t;
}
#endif

typedef std::pair<std::vector<std::string>::iterator,std::vector<std::string>::iterator> vsplit_ret_t;

static std::vector<vsplit_ret_t> vsplit(
std::vector<std::string>::iterator first
,const std::vector<std::string>::iterator &last
,const std::vector<std::string> &delim=std::vector<std::string>()
,unsigned int limit=0)
{
	std::vector<vsplit_ret_t> ret;
	if(delim.size()) for(auto i=first;i!=last;++i)
	{
		bool br = delim.size() == 0;
		//for(auto &d:delim) MS2010
		for(auto d=delim.begin();d!=delim.end();d++)
		{
			if(d->compare(*i)==0) { br=true; break; }
		}
		if(br)
		{
			if(limit && (ret.size()+1)==limit) break;
			if(i!=first) ret.push_back(vsplit_ret_t(first,i));
			first = i + 1;
		}
	}
	if(first!=last) ret.push_back(vsplit_ret_t(first,last));
	return ret;
}

static size_t skip(const char *str,const std::vector<std::string> &delim,size_t *ndelim=0)
{
	int ret = 0;
	if(ndelim) *ndelim = 0;
	if(str) for(;*str;)
	{
		size_t n = 0;
		auto d = delim.begin();
		for(;d!=delim.end();++d)
		{
			if(memcmp(str,d->c_str(),d->size())==0) 
			{
				n = d->size(); // not empty delim
				if(ndelim) *ndelim += 1;
				break;
			}
		}
		if(n==0) break;
		ret += n;
		str += n;
	}
	return ret; 
}

static size_t nskip(const char *str,const std::vector<std::string> &delim)
{
	int ret = 0;
	if(str) for(;*str;)
	{
		size_t n = 0;
		auto d = delim.begin();
		for(;d!=delim.end();++d)
		{
			if(memcmp(str,d->c_str(),d->size())==0) 
			{
				n = d->size(); // not empty delim
				break;
			}
		}
		if(n!=0) break;
		ret++;		
		str++;
	}
	return ret;
}

static inline bool npush(std::vector<std::string> &ret,const std::string &s,unsigned int limit)
{
	if(limit==0||ret.size()<limit)
	{
		ret.push_back(s);
		return true;
	}
	ret.back().append(s);
	return false;
}

static std::vector<std::string> vsplit(const char *str,const std::vector<std::string> &delim,int limit=0)
{
  std::vector<std::string> ret;
	--limit;
  if(str) for(;*str;)
  {
		size_t d;
    size_t n1 = skip(str,delim,&d);
    str += n1;
		while(limit==-1||(int)ret.size()<limit)
		{
			if(d-->1) ret.push_back(std::string()); else break;
		}
		if(limit==-1||(int)ret.size()<limit)
		{
	    size_t n2 = nskip(str,delim);
	    if(n2>0) ret.push_back(std::string(str, n2));
	    str += n2;
		}
		else
		{
	    if(*str) ret.push_back(std::string(str));
			break;
		}
  }
  return ret;
}
static std::vector<std::string> vsplit(const std::string &str,const std::vector<std::string> &delim,int limit=0)
{
	return vsplit(str.c_str(),delim,limit);
}

static std::vector<std::string> split(const char *str,const char *delim="\n",int limit=0)
{
  std::vector<std::string> ret;
	--limit;
  if(str) for(;*str;)
  {
    size_t n1 = strspn(str, delim);	
    str += n1;
		if(limit==-1||(int)ret.size()<limit)
		{
	    size_t n2 = strcspn(str, delim);
	    if(n2>0) ret.push_back(std::string(str, n2));
	    str += n2;
		}
		else
		{
	    if(*str) ret.push_back(std::string(str)); // allow delim
			break;
		}
  }
  return ret;
}
static inline std::vector<std::string> split(const std::string &str,const char *delim="\n",int limit=0)
{
	return split(str.c_str(),delim,limit);
}
static std::string join(const std::vector<std::string>::iterator &a,const std::vector<std::string>::iterator &b,const std::string &sep="\n")
{
  std::string ret;
	for(auto i=a;i!=b;i++) 
  {
		if(i!=a) ret += sep;
		ret += *i;
	}
	return ret;
}
static std::string join(const std::vector<std::string>::const_iterator &a,const std::vector<std::string>::const_iterator &b,const std::string &sep="\n")
{
  std::string ret;
	for(auto i=a;i!=b;i++) 
  {
		if(i!=a) ret += sep;
		ret += *i;
	}
	return ret;
}
static std::string join(const std::vector<const char*>::const_iterator &a,const std::vector<const char*>::const_iterator &b,const std::string &sep="\n")
{
  std::string ret;
	for(auto i=a;i!=b;i++) 
  {
		if(i!=a) ret += sep;
		ret += *i;
	}
	return ret;
}
static std::string join(const std::vector<std::string> &a,const std::string &sep="\n")
{
	return join(a.begin(),a.end(),sep);
}
static std::string join(const std::vector<const char*> &a,const std::string &sep="\n")
{
	return join(a.begin(),a.end(),sep);
}
static std::string resize(const std::string &a,size_t n,char c)
{
	if(a.size()==n) return a;
	std::string ret(a);
	ret.resize(n,c);
	return ret;
}
static std::string urlencode(const std::string &a)
{
	std::string ret;
	for(auto i=a.begin();i!=a.end();i++)
	{
		unsigned char c = *i;
		if(c==' ' || (!isalnum(c) && c!='-' && c!='_' && c!='.'))
		{
      static const char hex[] = "0123456789ABCDEF";
			char b[4];
      b[0] = '%';
      b[1] = hex[ c>>4 ];
      b[2] = hex[ c&0x0f ];
			b[3] = 0;
			ret.append(b);				
		}
		else
			ret.append(1,c);
	}
	return ret;
}
static int hex(char c)
{
  if(c>='0' && c<='9') return c-'0';
  if(c>='a' && c<='f') return c-'a' + 10;
  if(c>='A' && c<='F') return c-'A' + 10;
  return 0;	
}
static std::string urldecode(const std::string &a)
{
	std::string ret;
	for(auto i=a.begin();i!=a.end();i++)
	{
		unsigned char c = *i;
		if(c>127)
			ret.append(1,'?');
		else if(c=='+')
			ret.append(1,' ');
		else if(c=='%')
		{
			unsigned char h0 = *++i;
			if(i==a.end()) continue;
			unsigned char h1 = *++i;
			if(i==a.end()) continue;
			ret.append(1,(hex(h0)<<4)|hex(h1));
		}
		else
			ret.append(1,c);		
	}
	return ret;
}
static std::string substring(const std::string &s,std::string::size_type start,std::string::size_type end=std::string::npos)
{
	std::string::size_type len = end == std::string::npos ? std::string::npos : end > start ? end-start : 0;
	return s.substr(start,len);
}
static std::string replace(const std::string &s,const std::string &sub,const std::string &by)
{
	std::string ret;
	std::string::size_type i = 0, n = 0;
	while((n=s.find_first_of(sub,n))!=std::string::npos)
	{
		ret += substring(s,i,n);
		ret += by;
		i = n + sub.size();
		n = n + sub.size();		
	}
	ret += substring(s,i,n);
	return ret;
}
} // NS_(STR_H)

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

namespace STR_H {
  using std::pair;
  using std::string;
  using std::vector;
  using std::map;
  using std::multimap;
	template<typename T> std::string str(const T& a)
	{
		ostringstream ret;
		ret << a;
		return ret.str();
	}		
	static std::string str(int a,const char *fmt="%d")
	{
		return STD_H::sprintf(fmt,a);
	}
  static std::string str(const void *t, size_t len, int radix = 0, int bytes_per_line = 0, size_t maxlen = 80)
  {
    std::string ret;
		if(maxlen>len) maxlen=len;
    //ret += ::STD_H::sprintf("[%d%s]", len, len>maxlen?".":"");
		if(radix==0 && len>maxlen) len=maxlen;
    for (unsigned i = 0; i < len; i++)
    {
			if(radix!=0)
			{
	      if (bytes_per_line && (i%bytes_per_line)==0 && i!=(len-1)) ret += "\n";
	      if (i || bytes_per_line) if(radix!=1) ret += " ";
			}
      unsigned c = ((uchar*)t)[i];
      if (radix == 0)
        ret += ::STD_H::sprintf(c >= ' ' && c < 127 && c!='\\' ? "%c" : "\\x%02x", c);
      else if (radix == 1)
        ret += ::STD_H::sprintf(c >= ' ' && c < 127  ? "%c" : ".", c);
      else if (radix == 10)
        ret += ::STD_H::sprintf("%d", c);
      else 
        ret += ::STD_H::sprintf("%02x", c);
    }
		if(bytes_per_line) ret += "\n";
    return ret;
  }
	std::string str(const std::string &a, int radix = 0, int bytes_per_line = 0, size_t maxlen = 80)
	{
		return str(a.c_str(),a.size(),radix,bytes_per_line,maxlen);
	}		
  static std::string str(const sockaddr &a,int flag='a')
  {
		std::string host,port;
		const size_t alen=sizeof(sockaddr);
#ifdef NI_MAXHOST
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV]; // ws2tcpip.h
#else
    char hbuf[100], sbuf[100];
#endif
    if (getnameinfo(&a, alen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    {
			host = hbuf;
			port = sbuf;			
    }
#ifdef AF_INET
    else if (a.sa_family == AF_INET)
    {
      const sockaddr_in &i = (const sockaddr_in&) a;
      host = ::STD_H::sprintf("%d.%d.%d.%d", ((const unsigned char*) &i.sin_addr.s_addr)[0], ((const unsigned char*) &i.sin_addr.s_addr)[1], ((const unsigned char*) &i.sin_addr.s_addr)[2], ((const unsigned char*) &i.sin_addr.s_addr)[3]);
			port = ::STD_H::sprintf("%d",ntohs(i.sin_port));
    }
#endif
#ifdef AF_INET6
    else if (a.sa_family == AF_INET6)
    {
      host = "SOCKADDR_IN6";
    }
#endif
		else
		{
			host = ::STD_H::sprintf("SOCKADDR=%s", ::STR_H::str((void*)&a, alen).c_str());
		}
		std::string ret;
		if(flag=='a') ret += host + ':' + port;
		else if(flag=='h') ret = host;
		else if(flag=='p') ret = port;
		return ret;
  }
	template<typename T> std::string str(const std::vector<T> &a,const char *delim=",")
	{
		std::ostringstream ret;
    for(unsigned int i=0;i<a.size();i++) 
    {
      if(i) ret << delim;
			//ret << "[" << i << "]:";
      ret << str(a[i]);
    }
		return ret.str();
	}
	template<typename T,typename V> std::string str(const std::map<T,V> &a,const char *delim=",")
	{
		str::vector<std::string> ret;
		for(const auto &i:a)
    {
			std::ostringstream t;
      t << str(i.first) << ":" << str(i.second);
			ret.push_back(t.str());
    }
		return str(ret,delim);
	}
#if 0
  static std::string str(const char* fmt, va_list a) //GCCBUG (&a remove)
  {
    char s[4000];
    if (fmt) vsnprintf(s, sizeof (s), fmt, a);
    return s;
  }
  static std::string vstr(const char* fmt, va_list a) //GCCBUG (&a remove)
  {
    char s[4000];
    if (fmt) vsnprintf(s, sizeof (s), fmt, a);
    return s;
  }
#endif
} //NS_(STR_H)

namespace std { template<typename T1> ostream& operator<<(ostream& ret,const std::vector<T1> &a) { return ret << STR_H::str(a); } }
namespace std { template<typename T1,typename T2> ostream& operator<<(ostream& ret,const std::map<T1,T2> &a) { return ret << STR_H::str(a);} }

#endif
