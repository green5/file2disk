file2disk
=========

Map flash file to partition (file-disk matryoshka)
Why: You want test new OS and dont have enough disk space. Simply create file on disk and test it.

1. make or use existing fat32 partition  on flash disk (16G usually), labelit green, assume /dev/sdb1
   prepare boot file, f.e:
     dd if=/dev/zero of=/media/green/LINUX.IMG count=4 bs=1G
     mkfs.ext3 -L Linux /media/green/LINUX.IMG
2. file2disk /media/green/LINUX.IMG -A1  # create second active partition
3. eject/inject flash disk, check new partition 2
4. create boot image in first partition, f.e:
   grub-install --boot-directory=/media/Linux/boot /dev/sdb
   copy linux to /media/Linux/
5. try flash boot

Example:
<pre>
[:0.0]root@u3:boot[0]# df /dev/sdb1 /dev/sdb2
df: `/dev/sdb2': No such file or directory
Filesystem     1K-blocks      Used Available Use% Mounted on
/dev/sdb1       15672320   4450784  11221536  29% /media/green
[:0.0]root@u3:boot[0]# ./file2disk /media/green/LINUX.IMG -C1
/dev/sdb: 64 heads, 32 sectors, 15320 cylinders (HDIO_GETGEO)
/dev/sdb: 63 sectors 255 heads 929 cylinders (FromDisk)
Partitions:
Id              Boot Size        StartSector Sectors  Start   End        None
W95 FAT32 (LBA) 80   16064152064 63          31375297 1/1/0   63/254/928
Linux           00   4294967296  19282503    8388608  0/0/0   0/0/0
Empty           00   0           0           0        0/0/0   0/0/0
Empty           00   0           0           0        0/0/0   0/0/0
RootDir:
ShortName       Attr Size        StartSector Sectors  Cluster ChainsSize NChain
green           08   0                                0
GRLDR           21   239776      31383       469      44      245760     1
MENU.LST        21   2029        19282487    4        1203238 8192       1
LINUX.IMG       20   4294967295  19282503    8388608  1203239 4294967296 1
REGISTRY        10   0           34695       0        251     8192       1
[:0.0]root@u3:boot[0]# eject/inject flash disk, check new partition 2
[:0.0]root@u3:boot[0]# df /dev/sdb1 /dev/sdb2 
/dev/sdb1       15672320   4450784  11221536  29% /media/green
/dev/sdb2        4128444   3173988    744744  81% /media/Linux
</pre>

History:
05.02.2014: first release.

Problems:
Check disk is flash and really fat32.
