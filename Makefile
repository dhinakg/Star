CFLAGS?=-std=gnu99 -ffreestanding -O3 -Wall -Wextra -I./include
ARCH?=i686
TIME?=$(shell date +%s)

all:
	nasm -felf32 src/boot.asm -o boot.o
	nasm -felf32 driver/a20/check_a20.asm -o check_a20.o
	nasm -felf32 driver/a20/enable_a20.asm -o enable_a20.o
	nasm -felf32 driver/pic/disable.asm -o pic_disable.o
	$(ARCH)-elf-as driver/gdt/gdt.asm -o gdt_asm.o
	$(ARCH)-elf-as driver/idt/idt.asm -o idt_asm.o

	$(ARCH)-elf-gcc -c src/main.c -o main.o $(CFLAGS)
	$(ARCH)-elf-gcc -c src/tools.c -o tools.o $(CFLAGS)
	$(ARCH)-elf-gcc -c src/logging.c -o logging.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/vga.c -o vga.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/floppy.c -o floppy.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/pic/pic.c -o pic.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/gdt/gdt.c -o gdt.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/idt/idt.c -o idt.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/pit.c -o pit.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/nmi.c -o nmi.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/memory.c -o memory.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/serial.c -o serial.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/paging.c -o paging.o $(CFLAGS)
	$(ARCH)-elf-gcc -c driver/exceptions.c -o exceptions.o $(CFLAGS)

	$(ARCH)-elf-gcc -T linker.ld -o Star-$(ARCH).kernel -ffreestanding -O2 -nostdlib *.o -lgcc
	$(ARCH)-elf-objcopy --only-keep-debug Star-$(ARCH).kernel Star-$(ARCH).sym
	$(ARCH)-elf-objcopy --strip-debug Star-$(ARCH).kernel

	rm -rf *.o

test:
	qemu-system-x86_64 -kernel Star.kernel -m 32M -d guest_errors -fda DISK1.IMA

clean:
	rm -rf *.o *.bin *.kernel *.sym

all-arch:
	ARCH=i386 make
	ARCH=i486 make
	ARCH=i586 make
	ARCH=i686 make

	[[ -d build ]] || mkdir build
	mkdir build/$(TIME)
	cp *.kernel *.sym build/$(TIME)