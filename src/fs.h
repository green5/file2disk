#ifndef FS_H_
#define FS_H_

#include "include/std.h"
using STD_H::max2;

static int sum1(const string &a)
{
	uchar ret = 0;
	for(size_t i=0;i<a.size();i++) ret += (uchar)a[i];
	return ret;
}

#define vsum1(x) sum1(string((char*)&x,sizeof(x)))

struct Disk
{
	struct Entry // 16
	{
		uchar boot;
		uchar hd0;
		uint16_t sc0;
		uchar type;
		uchar hd1;
		uint16_t sc1;
		uint32_t begin;
		uint32_t size;
		friend ostream& operator<<(ostream& ret,const Entry &a)
		{
			return ret << STD_H::sprintf("boot=%d type=%d start=%d,%d end=%d,%d begin=%d size=%d"
				,a.boot,a.type,a.hd0,a.sc0,a.hd1,a.sc1,a.begin,a.size);
		}
		uint s0() const { return sc0 & 0x3F; }
		uint h0() const { return hd0; }
		uint c0() const { return (sc0>>8)+256*((sc0>>6)&3); }
		uint s1() const { return sc1 & 0x3F; }
		uint h1() const { return hd1; }
		uint c1() const { return (sc1>>8)+256*((sc1>>6)&3); }
		bool isfree() const
		{
			return type == 0;
		}
	} __attribute__((packed));
	char code[446];
	Entry entry[4];
	uint16_t signature;
	friend ostream& operator<<(ostream& ret,const Disk &a)
	{
		string c(a.code,sizeof(a.code));
		ret << STD_H::sprintf("pcode:%d",sum1(c));
		ret << str::str(a.code,sizeof(a.code),1,80,sizeof(a.code));
		ret << "0:" << a.entry[0] << "\n";
		ret << "1:" << a.entry[1] << "\n";
		ret << "2:" << a.entry[2] << "\n";
		ret << "3:" << a.entry[3] << "\n";
		ret << STD_H::sprintf("signature:%04xh",a.signature);
		return ret;
	}
	struct Geometry
	{
		uint32_t sectors,heads,cylinders;
		Geometry(const Disk &d)
		{
			sectors = heads = cylinders = 0;
			for(auto &a:d.entry) if(a.type!=0)
			{
				heads = max2(heads,(unsigned)a.hd0+1);
				heads = max2(heads,(unsigned)a.hd1+1);
				sectors = max2(sectors,a.s0());
				sectors = max2(sectors,a.s1());
				cylinders = max2(cylinders,a.c0()+1);
				cylinders = max2(cylinders,a.c1()+1);
			}
		}
		Geometry(const Geometry &a,size_t sect)
		{
			size_t u = a.heads*a.sectors;
			cylinders = sect / u;
			sect %= u;
			heads = sect / a.sectors;
			sect %= a.sectors;
			sectors = sect + 1;
		}
		uint16_t sc()
		{
			uint16_t ret = 0;
			ret |= sectors & 0x3f;
			cylinders &= 0x3FF;
			ret |= ((cylinders&255)<<8) | ((cylinders&3)<<6);
			return ret;
		}
		bool isValid()
		{
			return !(sectors==0 || heads==0 || cylinders==0);
		}
		friend ostream& operator<<(ostream&ret,const Geometry &a)
		{
			return cout << a.sectors << " sectors " << a.heads << " heads " << a.cylinders << " cylinders";
		}
	};
} __attribute__((packed));

#define vdump(x) str::str(string((char*)&x,sizeof(x)),1).c_str()
#define vdump2(x,n) str::str(string((char*)x,n),1).c_str()

// from dosutils:dosfsck.h
struct Dos
{
	struct DIR_ENT
	{
		uchar name[8], fext[3];			 /* name and extension */
		uchar attr;									 /* attribute bits */
		uchar lcase;								 /* Case for base and extension */
		uchar ctime_ms;							 /* Creation time, milliseconds */
		uint16_t ctime;							 /* Creation time */
		uint16_t cdate;							 /* Creation date */
		uint16_t adate;							 /* Last access date */
		uint16_t starthi;						 /* High 16 bits of cluster in FAT32 */
		uint16_t time, date, start;	 /* time, date and first cluster */
		uint32_t size;							 /* file size (in bytes) */
		string filename() const
		{
			string ret,ext;
			for(int i=0;i<8 && name[i]!=' ';i++) ret.push_back(name[i]);
			for(int i=0;i<3 && fext[i]!=' ';i++) ext.push_back(fext[i]);
			if(ext.size()) ext = "." + ext;
			return ret + ext;
		}
		uint32_t begin() const
		{
			return 0x10000*starthi + start;
		}
		friend ostream& operator<<(ostream& ret,const DIR_ENT &a)
		{
			return ret << STD_H::sprintf("%-12s %02x %10d %10d",a.filename().c_str(),a.attr,a.size,a.begin());
		}
	} __attribute__((packed));
	uchar jump[3];								 /* Boot strap short or near jump */
	uchar system_id[8];						 /* Name - can be used to special case partition manager volumes */
	uint16_t sector_size;					 /* bytes per logical sector, BPB start */
	uchar cluster_size;						 /* sectors/cluster */
	uint16_t reserved;						 /* reserved sectors */
	uchar fats;										 /* number of FATs */
	uint16_t dir_entries;					 /* root directory entries */
	uint16_t sectors;							 /* number of sectors */
	uchar media;									 /* media code (unused) */
	uint16_t fat_length;					 /* sectors/FAT */
	uint16_t secs_track;					 /* sectors per track */
	uint16_t heads;								 /* number of heads */
	uint32_t hidden;							 /* hidden sectors (unused) */
	uint32_t total_sect;					 /* number of sectors (if sectors == 0), BPB end */
	/* The following fields are only used by FAT32 */
	uint32_t fat32_length;				 /* sectors/FAT */
	uint16_t flags;								 /* bit 8: fat mirroring, low 4: active fat */
	uint16_t version;							 /* major, minor filesystem version */
	uint32_t root_cluster;				 /* first cluster in root directory, usually 2, 0 - free, -1 - last */
	uint16_t info_sector;					 /* filesystem info sector */
	uint16_t backup_boot;					 /* backup boot sector */
	uchar reserved2[12];					 /* Unused */
	uchar drive_number;						 /* Logical Drive Number */
	uchar reserved3;							 /* Unused */
	uchar extended_sig;						 /* Extended Signature (0x29) */
	uint32_t serial;							 /* Serial number */
	uchar label[11];							 /* FS label */
	uchar fs_type[8];							 /* FS Type */
	uchar code[420];
	uint16_t signature;
	friend ostream& operator<<(ostream& ret,const Dos &a)
	{
		ret << STD_H::sprintf("c=%d*%d res=%d %d*fat=%d"
			" root=%d/%d total=%d"
			" id=[%s] label=[%s] type=[%s] bcode=%d"
			,a.sector_size,a.cluster_size
			,a.reserved
			,a.fats,a.fat32_length
			,a.root_cluster,a.reserved+a.fat32_length*a.fats
			,a.sectors?a.sectors:a.total_sect
			,vdump(a.system_id),vdump(a.label),vdump(a.fs_type)
			,vsum1(a.code));
		return ret;
	}
} __attribute__((packed));
#endif
