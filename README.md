file2disk
=========

Map flash file to partition (generally disk matryoshka)

0. make fat32 on flash disk (16G usually), labelit myFlash, assume /dev/sdb1, format it
1. dd if=/dev/zero of=/media/myFlash/LINUX.IMG count=4 bs=1G
2. mkfs.ext3 -L Linux /media/myFlash/LINUX.IMG

or use grub, syslinux flash

3. file2disk /media/myFlash/LINUX.IMG -A1 (create second active partition)
3.1 file2disk /dev/sdb someBoot.img (not done)
4. eject/inject flash disk
5. check new partition
6. grub-install --boot-directory=/media/Linux/boot /dev/sdb, check grub.cfg under that boot/grub/ (UUID,...)
7. copy Linux ... (cd /someRoot; cp -a * /media/Linux/; umount /dev/sdb*); or something else
8. boot from flash

History:
05.02.2014: first release.

Problems:
Check disk is flash and really fat32.
Add 3.1



