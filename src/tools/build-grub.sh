#!/bin/sh

if [ `id -u` != 0 ]; then
	echo "only root can do this";
	exit
fi

echo Please insert a blank 1.44MB floppy in /dev/fd0 and hit return to continue...
echo or ctrl-c to quit
read a

/sbin/mkfs -V -t msdos /dev/fd0
mount /dev/fd0 /mnt
mkdir -p /mnt/boot/grub
cp /usr/local/share/grub/i386-pc/stage1 /mnt/boot/grub
cp /usr/local/share/grub/i386-pc/stage2 /mnt/boot/grub
echo '# by default, boot the first entry
default 0

# boot automatically after 2 seconds
timeout 2

# normal bootup
title XeOS
kernel --type=netbsd /boot/xeos
' > /mnt/boot/grub/grub.cfg
umount /mnt
echo 'root (fd0)
install /boot/grub/stage1 d (fd0) (fd0)/boot/grub/stage2 0x8000 p (fd0)/boot/grub/grub.cfg
quit
' | /usr/sbin/grub --batch

dd if=/dev/fd0 of=base-grub.flp bs=18k
