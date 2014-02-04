#ifndef MISC_H
#define MISC_H

// -lrt

#ifdef __unix__
#include <syscall.h>
#endif

namespace STD_H
{
#ifdef _WIN32
	static int cpucount()
	{
	  SYSTEM_INFO sysinfo;
	  GetSystemInfo( &sysinfo );
	  return sysinfo.dwNumberOfProcessors;
	}
	static double ns()
	{
		SYSTEMTIME t;
		GetSystemTime(&t);
		FILETIME f;
		SystemTimeToFileTime(&t,&f);		
		ULARGE_INTEGER d;
		d.LowPart = f.dwLowDateTime;
		d.HighPart = f.dwHighDateTime;
		return (double)d.QuadPart/1.e7;
	}
#endif
#ifdef __unix__
	static int cpucount()
	{
	  return sysconf(_SC_NPROCESSORS_ONLN);
	}
#if 0
	static double ns()
	{
		struct timespec t;
		clock_gettime(0,&t);
		return t.tv_nsec/1000000000.+t.tv_sec;
	}
#endif
	static int nthread()
	{
		int ret = 0;
	  pid_t pid = getpid();
		DIR *dir = opendir(STD_H::sprintf("/proc/%d/task",(int)pid).c_str());
		if(dir)
		{
			dirent *e;
			while((e=readdir(dir))!=0)
			{
				if(!strcmp(e->d_name,".")) continue;
				if(!strcmp(e->d_name,"..")) continue;
				ret++;
			}
			closedir(dir);
		}
		return ret;
	}
#endif
	static string threadid()
	{
		long x;
#ifdef __linux
		x = (long)(pid_t)syscall(SYS_gettid);
#elif defined(_WIN32)
		x = GetCurrentThreadId();
#else
		x = (long)pthread_self();
#endif
		return STD_H::sprintf("%ld",x);
	}
}

#endif
