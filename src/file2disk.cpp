// Copyright 2011 Rustem Valeev <r-green@mail.ru>

//#define TRACE_H
#include "include/main.h"
#include "include/io.h"
#include <iostream>
#include <iomanip>

#include "fs.h"
#include "fd.h"

using STD_H::max2;
using STD_H::min2;

struct Grid
{
	vector<string> data;
	template<typename T> friend Grid& operator<<(Grid &ret,const T &a)
	{
		ostringstream t;
		t << a;
		ret.data.push_back(t.str());
		return ret;
	}
	friend Grid& operator<<(Grid &ret,const char &a)
	{
		return ret << STD_H::sprintf("%s%d",a>0?"+":"",a);
	}		
	friend Grid& operator<<(Grid &ret,const unsigned char &a)
	{
		return ret << STD_H::sprintf("%02x",a);
	}		
	string str(size_t ncol=1)
	{
		std::ostringstream ret;
		vector<size_t> w(ncol,0);		
		for(size_t i=0;i<data.size();)
		{
			w[i%w.size()] = max2(w[i%w.size()],data[i].size());
			i++;
		}
		for(size_t i=0;i<data.size();)
		{
			ret << std::left;
			ret << std::setw(w[i%w.size()]+1) << data[i];
			i++;
			if((i%w.size())==0) ret << std::endl;
		}
		data.clear();
		return ret.str();		
	}
};

#include "i386_sys_types.c"
#include <linux/hdreg.h>

void print_geometry(const string &bdev,DskFd &b)
{
#ifdef HDIO_GETGEO
	struct hd_geometry geometry;
	if (!ioctl(b.fd, HDIO_GETGEO, &geometry)) 
	{
		cout << STD_H::sprintf("%s: %d heads, %d sectors, %d cylinders (HDIO_GETGEO)\n",bdev.c_str(),geometry.heads,geometry.sectors,geometry.cylinders);
	}
#endif
	Disk::Geometry g(b.disk);
	cout << bdev << ": " << g << " (FromDisk)\n";
}

void print(Grid &out,DskFd &b)
{
	out << "Id";
 	out << "Boot";
 	out << "Size";
	out << "StartSector";
	out << "Sectors";
	out << "Start";
	out << "End";
	out << "None";
	for(auto &a:b.disk.entry)
	{
		if(i386_sys_types[a.type]) out << i386_sys_types[a.type]; else out << a.type;
		out << a.boot;
		out << 512L*a.size;
		out << a.begin;
		out << a.size;
		out << STD_H::sprintf("%d/%d/%d",a.s0(),a.h0(),a.c0());
		out << STD_H::sprintf("%d/%d/%d",a.s1(),a.h1(),a.c1());
		out << "";
	} 
}

void print(Grid &out,DosFd &a)
{
	out << "ShortName";
	out << "Attr";
	out << "Size";
	out << "StartSector";
	out << "Sectors";
	out << "Cluster";
	out << "ChainsSize";
	out << "NChain";
	for(auto &e:a.readRoot()) 
	{
		out << e.filename();
		out << e.attr;
		out << e.size;
		uint32_t b = e.begin();
		if(b>=2 && b<a.nfat1)
		{
			off_t sector_ = a.sector(b)+a.pstart_; // StartSector
			size_t chain_size;
			uint32_t nchain = a.nchain(b,&chain_size);
			out << sector_;
			out << (e.size+511L)/512;
			out << e.begin();
			out << chain_size;
			out << nchain;
		}
		else
			out << "" << "" << e.begin() << "" << "";
	}
}

void print(const string &bdev,DskFd &b,DosFd &a)
{
	Grid out;
	print_geometry(bdev,b);
	print(out,b);
	print(out,a);
	cout << out.str(8);
}

string findDisk(const char *apath)
{
	struct stat st;
	if(stat(apath,&st)==-1) pexit(apath);
	if(!S_ISREG(st.st_mode)) pexit("%s: not regular file\n",apath);
	char &major = ((char*)&st.st_dev)[1];
	char &minor = ((char*)&st.st_dev)[0];
	plog("%s: dev %d,%d size %ld sectors",apath,major,minor,(long)(st.st_size+511L)/512);
	string dev = IO_H::finddev(st.st_dev);
	if(!dev.size()) pexit("%s: cant find block device",apath);
	for(auto i=dev.end();--i!=dev.begin() && isdigit(*i);) *i=0;
	return dev.c_str();
}

#include <libgen.h>

int filecmp(const char *apath,DosFd &a,Dos::DIR_ENT &e,size_t total,size_t size)
{
	int ret = 0;
	struct stat st;
	passert(size>0);
	if(stat(apath,&st)==-1) pexit(apath);
	int fd1 = open(apath,O_RDONLY);
	if(fd1==-1) pexit(apath);
	total = min2(total,(size_t)st.st_size);
	for(size_t i=0;i<total;i+=size)
	{
		string s2 = a.preadFile(e.begin(),size,i);
		size_t n = s2.size();
		char t[n];
		if((size_t)::read(fd1,t,n)!=n) 
		{
			ret = -1;
			break;
		}
		ret = memcmp(t,s2.c_str(),n);
		if(ret) break;
	}
	close(fd1);
	return ret;
}

int ask(const char *prompt)
{
	fputs(prompt,stdout);
	fflush(stdout);
	char ask[10]; 
	ask[0] = 0;
	fgets(ask,sizeof(ask),stdin);
	return ask[0];
}

void file2disk(const char *apath,char mode,int part)
{
	string bdev = findDisk(apath);
	if(!bdev.size()) PEXIT;
	DskFd b(bdev);
	const Disk::Entry &e0 = b.disk.entry[0];
	if(e0.isfree()) PEXIT;
	DosFd a(b.fd,e0.begin);
	if(mode==0) 
	{
		print(bdev,b,a);
		return;
	}
	string name;
	if(1==1)
	{
		char t[strlen(apath)+1];
		strcpy(t,apath);
		name = basename(t);
	}
	Dos::DIR_ENT e;
	if(1==1) // check file (main part)
	{
		if(!a.findFile(name,e)) pexit("%s: can't find in rootdir of first partition",apath,bdev.c_str());	
		uint32_t nchain = a.nchain(e.begin());
		passert(nchain==1,"file fragmented, nchain=%ld",(long)nchain);
		int x = filecmp(apath,a,e,3000,a.clusterSize());
		if(x) pexit("%s: path and disk files differ(%d)",apath,x);
	}
	if(part==0)
	{
		for(auto &t:b.disk.entry)	
		{
			if(t.isfree()) { part = &t - b.disk.entry; break; }
		}
		if(part==0) pexit("%s: no free partitions",bdev.c_str());
	}
	if(!(part>0&&part<4)) pexit("%d: bad partition, start from zero",part);
	Disk::Entry &f = b.disk.entry[part];
	if(!f.isfree()) 
	{
		print(bdev,b,a);
		perr("%d: busy partitions",part);
		if(ask("Continue[Ny]?")!='y') return;
	}
	f.boot = mode == 'A' ? 0x80 : 0;
	f.type = 0x83; // Linux
	f.begin = a.sector(e.begin())+a.pstart_;
	f.size = (e.size+511L)/512; /// or chain size? //// L?
	//plog("%ld %ld",f.size,e.size);
	f.sc0 = 0;
	f.sc1 = 0;
	f.hd0 = 0;
	f.hd1 = 0;
	if(1==0) ///
	{
		Disk::Geometry g(b.disk);		
		passert(g.isValid());
		Disk::Geometry g0(g,f.begin);
		Disk::Geometry g1(g,f.begin+f.size-1);
		f.sc0 = g0.sc();
		f.hd0 = g0.heads;
		f.sc1 = g1.sc();
		f.hd1 = g1.heads;
	}
	print(bdev,b,a);
	if(mode=='A'||mode=='C')
	{
		if(ask("Commit changes[Ny]?")=='y')
		{
			int fd = open(bdev.c_str(),O_WRONLY);
			ssize_t n = write(fd,&b.disk,512);
			fsync(fd);
			close(fd);
			if(n!=512) perror("MBR"),PEXIT;
			plog("commit = %ld",(long)n);
		}
	}
}

void usage(int ac,char *av[])
{
	printf(
		"usage: file2disk file [-[AC][0123]]\n"
		"example: %s /media/myFlash/GRLDR -A2 (make active partition pointing to file)\n"
		,av[0]);
	exit(1);
}
int main(int ac,char *av[])
{
	//STD_H::opt::main(ac,av);
	if(!(ac==2||ac==3)) usage(ac,av);
	char *path = av[1];
	if(!strcmp(path,"--help")) usage(ac,av);
	int part = 0; // auto
	int mode = 0; 
	for(const char *a =(ac==3 && av[2][0]=='-')?av[2]:"";*a;a++)
	{
		if(isupper(*a)) mode = *a;
		if(isdigit(*a)) part = *a - '0';
	}
	file2disk(path,mode,part);
	return 0;
}
