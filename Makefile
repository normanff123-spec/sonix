CC = gcc
AS = nasm
CFLAGS = -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -ffreestanding -I kernel -mno-sse -mno-sse2
LDFLAGS = -T linker.ld -melf_i386
ASFLAGS = -f elf32
OBJS = boot.o kernel.o interrupt.o
all: myos.iso
kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) kernel/kernel.c -o kernel.o
boot.o: kernel/boot.s
	$(AS) $(ASFLAGS) kernel/boot.s -o boot.o
interrupt.o: kernel/interrupt.s
	$(AS) $(ASFLAGS) kernel/interrupt.s -o interrupt.o
myos.bin: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o myos.bin
myos.iso: myos.bin
	mkdir -p iso/boot/grub
	cp myos.bin iso/boot/
	cp boot/grub/grub.cfg iso/boot/grub/
	grub-mkrescue -o myos.iso iso
run: myos.iso
	qemu-system-i386 -cdrom myos.iso -netdev user,id=n1 -device e1000,netdev=n1 -m 128
run-non: myos.iso
	qemu-system-i386 -cdrom myos.iso -m 128
clean:
	rm -rf *.o *.bin iso myos.iso
