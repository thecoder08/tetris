run: tetrisos.img
	kvm --gdb tcp::1234 -m 1G -hda $^

tetrisos.img: tetrisos.elf grub.cfg tetrisos.sfdisk
	dd if=/dev/zero of=$@ bs=1M count=10
	cat tetrisos.sfdisk | sfdisk $@
	sudo losetup -P loop8 $@
	sudo mkfs.fat -F16 -n TETRISOS /dev/loop8p1
	mkdir build
	sudo mount /dev/loop8p1 build
	sudo cp tetrisos.elf build
	sudo grub-install --boot-directory=build --target=i386-pc /dev/loop8
	sudo cp grub.cfg build/grub
	sudo umount build
	sudo losetup -d /dev/loop8
	rmdir build

tetrisos.elf: multiboot_gdt.o gdt.o graphics.o interrupts.o io.o ps2.o tetris-os.o tiles.o
	ld $^ -melf_i386 -e kmain -Ttext 0x0 -o $@

%.o: %.asm
	nasm -g -f elf32 $^

%.o: %.c
	gcc -ffreestanding -g -m32 -Wall -c $^ -mgeneral-regs-only -fno-pie -fno-stack-protector

clean:
	rm -rf *.o *.elf *.img build