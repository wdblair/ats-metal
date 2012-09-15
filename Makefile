CC	= i586-elf-gcc
CFLAGS	= -Wall -Wextra -nostdlib -ffreestanding -nostartfiles -nodefaultlibs
LD	= i586-elf-ld

OBJFILES = loader.o kernel.o

all: kernel.img

.s.o:
	nasm -f elf -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

kernel.bin: $(OBJFILES)
	$(LD) -T linker.ld -o $@ $^

kernel.img: kernel.bin
	dd if=/dev/zero of=pad bs=1 count=750
	cat stage1 stage2 pad $< > $@
	du --apparent-size -B 512 $<

clean:
	$(RM) $(OBJFILES) kernel.bin kernel.img

install:
	$(RM) $(OBJFILES) kernel.bin
