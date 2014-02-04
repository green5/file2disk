#ifndef FD_H
#define FD_H

static void read1(int fd,off64_t off,void *ret,size_t size)
{
	auto o = lseek64(fd,off,SEEK_SET);
	if(o==-1) pexit("lseek");
	ssize_t n = ::read(fd,ret,size);
	if((size_t)n!=size) pexit("read");
}

static void read512(int fd,off64_t off,void *ret,size_t size)
{
	read1(fd,off*512,ret,size*512);
}

struct Fd
{
	int fd;
	Fd(int fd_):fd(dup(fd_))
	{
	}
	Fd(const string &a)
	{
		fd = IO_H::open(a,O_RDONLY);
		if(fd==-1) pexit("%s:open",a.c_str());
	}
	virtual ~Fd()
	{
		close(fd);
	}
};

struct DskFd : Fd
{
	Disk disk;
	template<typename T> DskFd(const T& a,off_t sector=0):Fd(a)
	{
		passert(sizeof(disk)==512);
		read512(fd,sector,&disk,1);
		passert(disk.signature==0xaa55);
	}
};

// http://www.easeus.com/resource/fat32-disk-structure.htm
// http://www.pjrc.com/tech/8051/ide/fat32.html
struct DosFd : Fd
{
	off_t pstart_; // partition
	Dos boot;
	uint32_t *fat1; 
	uint32_t nfat1;
	void read512(int fd,off64_t off,void *ret,size_t size) const
	{
		::read512(fd,pstart_+off,ret,size);
	}
	template<typename T> DosFd(const T& a,off_t sector):Fd(a),pstart_(sector)
	{
		passert(sizeof(boot)==512);
		passert(sizeof(Dos::DIR_ENT)==32);
		passert(pstart_!=0);
		read512(fd,0,&boot,1);
		passert(boot.signature==0xaa55);
		passert((boot.sector_size%512)==0);
		passert(boot.fats && boot.fat32_length);
		nfat1 = boot.fat32_length*(512/4);
		fat1 = new uint32_t[nfat1];
		read512(fd,boot.reserved,fat1,boot.fat32_length);
	}
	~DosFd()
	{
		delete[] fat1;
	}
	void travel(uint32_t start_,std::function<bool(uint32_t,uint32_t)> Func) const
	{
		uint32_t start = start_;
		passert(start,"%08lxh",start); // not free 
		passert(start>=2,"%08lxh",start); // data
		passert(start<nfat1,"%08lxh<%08lxh",start,nfat1);
		while(1)
		{
			//plog("%08lx:%08lx",start_,start);
			uint32_t next = fat1[start];
			if(!Func(start,next)) break;
			if(next>=(uint32_t)0x0FFFFFF8) break;
			passert(next!=start,"%08lxh!=%08lxh",next,start);
			passert(next<nfat1,"%08lxh<%08lxh",next,nfat1);
			start = next;
		}
	}
	vector<uint32_t> chain(uint32_t start) const
	{
		vector<uint32_t> ret;
		travel(start,[&ret](uint32_t start,uint32_t next)->bool
		{
			ret.push_back(start);
			return true;
		});
		return ret;
	}
	off_t sector(uint32_t start) const
	{
		return boot.reserved + boot.fat32_length*boot.fats + (start-2)*boot.cluster_size;
	}
	size_t clusterSize() const
	{
		return (size_t)boot.sector_size*boot.cluster_size;
	}
	uint32_t nchain(uint32_t start,size_t *size=0) const
	{
		uint32_t ret = 0;
		if(size) *size  = 0;
		travel(start,[&](uint32_t start,uint32_t next)
		{
			if(size) *size += clusterSize();
			if((start+1)!=next) ret++;
			return true;
		});
		return ret;
	}
	string readFile(uint32_t astart)
	{
		string ret;
		size_t block = clusterSize();
		travel(astart,[&](uint32_t start,uint32_t next)
		{
			char t[block];
			read512(fd,sector(start),t,sizeof(t)/512);
			ret.append(t,sizeof(t)); /// skip copy
			return true;
		});
		return ret;
	}
	string preadFile(uint32_t astart,size_t count_,off_t offset)
	{
		string ret;
		size_t block = clusterSize();
		size_t count = count_;
		off_t skip = offset;
		off_t file_offset = 0;
		uint32_t fstart = 0;
		travel(astart,[&](uint32_t start,uint32_t next)
		{
			if(skip<(off_t)block) 
			{
				char t[block];
				read512(fd,sector(start),t,sizeof(t)/512);
				size_t n = block - skip;
				if(count<n) n = count;
				ret.append(t+skip,n);
				count -= n;
				skip = 0;
				file_offset += block;
				fstart = next;
				return false;
			}
			file_offset += block;
			skip -= block;
			return true;
		});
		if(count==0) return ret; // done
		if(fstart==0) return ret; // error
		travel(fstart,[&](uint32_t start,uint32_t next)
		{
			char t[block];
			read512(fd,sector(start),t,sizeof(t)/512);
			if(count<block)
			{
				ret.append(t,count);
				return false;
			}			
			ret.append(t,block);
			count -= block;
			return true;					
		});
		passert(ret.size()==count_);
		return ret;
	}
	vector<Dos::DIR_ENT> readRoot() const
	{
		vector<Dos::DIR_ENT> ret;
		travel(boot.root_cluster,[&](uint32_t start,uint32_t next)
		{
			Dos::DIR_ENT t[(boot.cluster_size*boot.sector_size)/sizeof(Dos::DIR_ENT)];
			off_t n = sector(start);
			read512(fd,n,t,sizeof(t)/512);
			for(size_t i=0;i<sizeof(t);i++)
			{
				Dos::DIR_ENT &e = t[i];
				if(e.name[0]==0x00) return false;
				if(e.name[0]==0xE5) continue;
				if((e.attr&0x0F)==0x0F) continue; /// LFN
				if(e.name[0]==0x05) e.name[0]=0xE5;
				ret.push_back(e);
			}			
			return true;
		});
		return ret;
	}
	bool findFile(const string &name,Dos::DIR_ENT &ret)
	{
		int n = 0;
		for(auto &e:readRoot())	
		{
			if(!strcasecmp(name.c_str(),e.filename().c_str()))
			{
				n++;
				ret = e;
			}
		}
		return n==1;
	}
};

#endif
