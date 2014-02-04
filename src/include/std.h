#ifndef STD_H
#define STD_H my

// Copyright 2011 Rustem Valeev <r-green@mail.ru>
//
// std.h is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// std.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with std.h; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#ifdef __unix__
#include <netinet/ip.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#endif

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <functional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <limits.h>
#include <cstdio>
#include <cstdarg>

#if 0
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#endif

#define NS(A) namespace A {
#define NT(A) };
#define NS_(A) };
#define NS2(A,B) }; namespace B {

typedef unsigned char uchar;
#ifdef _WIN32
typedef unsigned int uint;
typedef unsigned long ulong;
#endif

using std::string;
using std::vector;
using std::map;
using std::ostream;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::stringbuf;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::cin;
using std::cout;
using std::cerr;

#ifndef vsizeof
#define vsizeof(v) (sizeof(v)/sizeof(v[0]))
#endif

#ifndef DEBUGGER
#define DEBUGGER asm("int $3")
#endif

namespace STD_H
{
	static std::string vsprintf(const char *fmt,va_list v) 
	{
		string::size_type n = 4096;
		std::string t(n,0);
		n = vsnprintf((char*)t.c_str(),n,fmt,v);
		t.resize(n);		
		return t;
	}
	static std::string vprintf(const char *fmt,va_list v) 
	{
		string::size_type n = 4096;
		std::string t(n,0);
		n = vsnprintf((char*)t.c_str(),n,fmt,v);
		t.resize(n);		
		return t;
	}
	static std::string printf(const char *fmt,...) 
	{
		va_list a;
		va_start(a,fmt);
		std::string ret = STD_H::vsprintf(fmt,a);
		va_end(a);
		return ret;
	}
	static std::string sprintf(const char *fmt,...) 
	{
		va_list a;
		va_start(a,fmt);
		std::string ret = STD_H::vsprintf(fmt,a);
		va_end(a);
		return ret;
	}
	typedef std::function<void(string&)> fout_t;
#ifdef MAIN_H
	fout_t fout_ = 0;
#else
	extern fout_t fout_;
#endif
}

namespace STD_H //misc
{
	static double ns();
	static int cpucount();
	static int nthread();
	static string threadid();
}

#ifdef __GNUC__
#define noinline __attribute__((noinline))
#else
#define noinline
#define noexcept
#endif

#ifndef plog
namespace STD_H
{
	struct pexception : std::exception
	{
		std::string w_;
		pexception(const std::string &w):w_(w){}
		virtual ~pexception() throw() {}
		const char* what() const noexcept {return w_.c_str();}
	};
}
struct Line_
{
  const char *file_;
  const int line_;
  const char *func_;
	STD_H::fout_t fout_;
  noinline Line_(const char *file, const int line, const char *func,int)
  : file_(file), line_(line), func_(func)
  {		
		fout_ = STD_H::fout_ ? STD_H::fout_ : [](string &s)
		{
			if(::write(1,s.c_str(),s.size())==-1) {}
		};
  }
  noinline Line_(const char *file, const int line, const char *func,const char *out)
  : file_(file), line_(line), func_(func)
  {
		fout_ = [&out](string &s)
		{
			int out_ = open(out,O_APPEND|O_CREAT);
			(void)write(out_,s.c_str(),s.size());
			if(out_>2) close(out_);
		};
  }
  string hstr() const
  {
    std::ostringstream out;
		if(1==1)
		{
			time_t tt = time(0);
			struct tm t;
#if defined(_WIN32) && !defined(__GNUC__)
			gmtime_s(&t,&tt);
#else
			gmtime_r(&tt,&t);
#endif
			out << STD_H::sprintf("%02d:%02d:%02d",t.tm_hour,t.tm_min,t.tm_sec);
		}
		//out << "[" << getpid() << "]";
		//out << "[" << &out << "]";
		out << "[" << STD_H::threadid() << "]";
    out << file_ << "." << line_ << "." << func_ << ": ";
    return out.str();
  }
  void fout(const std::string &b)
  {
		std::string c = hstr() + b + "\n";
		if(fout_!=0) fout_(c);
  }
  int pthrow_(const string &a)
  {
    string t = hstr() + a;
		if(lastchar(a) != '.' && errno) t += " (" + error() + ")";
    throw ::STD_H::pexception(t);
		return -1;
  }
  int pthrow_(const char *fmt, ...)
  {
    va_list a;
    va_start(a, fmt);
    string t = hstr() + STD_H::vsprintf(fmt, a);
		if(lastchar(fmt) != '.' && errno) t += " (" + error() + ")";
    va_end(a);
    throw ::STD_H::pexception(t);
		return -1;
  }
  void passert_(bool bo, const char *fmt=0, ...)
  {
    if(bo) return;
    va_list a;
    va_start(a,fmt);
    string t = hstr() + STD_H::vsprintf(fmt?fmt:"no description", a);
		if(lastchar(fmt) != '.' && errno) t += " (" + error() + ")";
    va_end(a);
    throw ::STD_H::pexception(t);
  }
  void passert_(bool bo,const string &a)
  {
    if(bo) return;
    string t = hstr() + a;
    throw ::STD_H::pexception(t);
  }
  noinline int plog_(const char *fmt, ...)
  {
    va_list a;
    va_start(a, fmt);
    fout(STD_H::vsprintf(fmt, a));
    va_end(a);
    return -1;
  }
  noinline int plog_(const string &a) //TODO: cstring use
  {
		fout(a);
    return -1;
  }
  int lastchar(const char *a)
  {
    return a ? a[strlen(a) - 1] : 0;
  }
  int lastchar(const string &a)
  {
    return a.size() ? a[a.size() - 1] : 0;
  }
	string error()
	{
#ifdef __unix__
		return strerror(errno);
#endif
#ifdef _WIN32
		return STD_H::sprintf("winerror-%ld",(long)GetLastError());
#endif		
	}
  noinline int perr_(const string &a)
  {
		string t = a;
		if(lastchar(t) != '.') t += " (" + error() + ")";
    fout(t);
    return errno ? errno : 123;
  }
  noinline int perr_(const char *fmt = 0, ...)
  {
    va_list a;
    va_start(a, fmt);
		string t = STD_H::vsprintf(fmt, a);
    int ret = perr_(t);
    va_end(a);
    return ret;
  }
  void pexit_(const string &a)
  {
		string t = a;
		if(lastchar(t) != '.') t += " (" + error() + ")";
    fout(t);
#ifdef TRACE_H
		DEBUGGER;
#endif
    exit(errno?errno:1);
  }
  void pexit_(const char *fmt = 0, ...)
  {
    if(!fmt) fmt = "EXIT";
    va_list a;
    va_start(a, fmt);
		string t = STD_H::vsprintf(fmt, a);
		pexit_(t);
  }
  int pdebug_(const char *fmt, ...)
  {
    va_list a;
    va_start(a, fmt);
    fout(STD_H::vsprintf(fmt, a));
    //asm("int $3");
    return -1;
  }
	template<typename T> inline T nval(T a,const char *fmt=0,...)
	{
		if(a==0) {va_list v;va_start(v,fmt);fout("NULL");va_end(v);}
		return a;
	}	
	template<typename T> inline T zval(T a,const char *fmt=0,...)
	{
		if(a!=0) {va_list v;va_start(v,fmt);fout("NOT NULL");va_end(v);}
		return a;
	}	
	template<typename T> inline T tval(T a,const char *fmt=0,...)
	{
		if(!a) {va_list v;va_start(v,fmt);fout("FALSE");va_end(v);}
		return a;
	}	
	template<typename T> inline T fval(T a,const char *fmt=0,...)
	{
		if(!!a) {va_list v;va_start(v,fmt);fout("TRUE");va_end(v);}
		return a;
	}	
};
#define LINE_(x) Line_(__FILE__,__LINE__,__FUNCTION__,x)
#define passert LINE_(2).passert_
#define pthrow  LINE_(2).pthrow_
#define perr    LINE_(2).perr_
#define plog    LINE_(2).plog_
#define pexit   LINE_(2).pexit_
#define pdebug  LINE_(2).pdebug_
#if 0
#define ptrace LINE_(2).pdebug_
#define ftrace LINE_("ptrace.log").pdebug_
#else
#define ptrace(fmt,...)
#define ftrace(fmt,...)
#endif
#define PLINE		LINE_(2).hstr()
#define PLOG 		plog("LOG")
#define PERR 		perr("ERR")
#define PDBG 		pdebug("DBG")
#define PEXIT		pexit("EXIT")
#define PTHROW	pthrow("THROW")
#define nVAL 		LINE_(2).nval
#define zVAL 		LINE_(2).zval
#define tVAL 		LINE_(2).tval
#define fVAL 		LINE_(2).fval
#endif //PLOG

namespace STD_H
{
  template<typename T1, typename T2> static inline T1 min2(T1 t1, T2 t2)
  {
    return t1 <= t2 ? t1 : t2;
  }
  template<typename T1, typename T2> static inline T1 max2(T1 t1, T2 t2)
  {
    return t1 >= t2 ? t1 : t2;
  }
  template<typename S> static void swap(S &a, S &b)
  {
    S t = a;
    a = b;
    b = t;
  }
}

#include "str.h"
#include "misc.h"

#ifdef WOBJECT_H_
#include "wt.h"
#endif

#endif
