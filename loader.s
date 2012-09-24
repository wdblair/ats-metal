[BITS 32]       ; 32 bit code
[section .text] ; keep NASM happy
[global start]  ; make 'start' function global
[global magic]
[extern kmain]  ; our C kernel main
 
; Multiboot constants
MULTIBOOT_PAGE_ALIGN	equ 1<<0
MULTIBOOT_MEMORY_INFO	equ 1<<1
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
 
; Multiboot header (needed to boot from GRUB)
ALIGN 4
multiboot_header:
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM
 
; the kernel entry point
start:
	; here's the trick: we load a GDT with a base address
	; of 0x40000000 for the code (0x08) and data (0x10) segments
	lgdt [trickgdt]
        mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
        
 	; jump to the higher half kernel
	jmp 0x08:higherhalf
 
higherhalf:
	; from now the CPU will translate automatically every address
	; by adding the base 0x40000000
	mov esp, sys_stack ; set up a new stack for our kernel
	call kmain ; jump to our C kernel ;)
 
	; just a simple protection...
	jmp $
 
[global gdt_flush] ; make 'gdt_flush' accessible from C code
[extern gp]        ; tells the assembler to look at C code for 'gp'
 
; this function does the same thing of the 'start' one, this time with
; the real GDT
gdt_flush:
	lgdt [gp]
        mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2
 
flush2:
	ret
 
[section .setup] ; tells the assembler to include this data in the '.setup' section
 
trickgdt:
	dw gdt_end - gdt - 1 ; size of the GDT
	dd gdt ; linear address of GDT
 
gdt:
	dd 0, 0							; null gate
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40	; code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40	; data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
 
gdt_end:
 
[section .bss]

align 4
resb 0x4000
sys_stack:
resd 1
magic:  
