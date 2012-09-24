#include <stdint.h>
#include "multiboot.h"

// Defines the structures of a GDT entry and of a GDT pointer
struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));
 
struct gdt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));
 

// We declare a pointer to the VGA array and its attributes
unsigned short *video = (unsigned short *)0xC00B8000; // We could also use the virtual address 0xC00B8000
unsigned char attrib = 0xF; // White text on black background

static unsigned int xpos, ypos;

#define LINES 25
#define COLUMNS 80

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal. */
static void
itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
     
  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
     
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

/* Put the character C on the screen. */
static void
putchar (int c)
{
  if (c == '\n' || c == '\r')
    {
    newline:
      xpos = 0;
      ypos++;
      if (ypos >= LINES)
        ypos = 0;
      return;
    }
     
  *(video + (xpos + ypos * COLUMNS)) = (attrib << 8) | c & 0xFF;

  xpos++;
  if (xpos >= COLUMNS)
    goto newline;
}

void puts(char *s) {
  while(*s) {
    putchar(*s++);
  }
  return;
}

// We'll need at least 3 entries in our GDT...
 
struct gdt_entry gdt[3];
struct gdt_ptr gp;
 
// Extern assembler function
void gdt_flush();
 
// Very simple: fills a GDT entry using the parameters
void gdt_set_gate(int num, unsigned long base, unsigned long limit, 
                  unsigned char access, unsigned char gran)
{
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);
  
  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}

// Sets our 3 gates and installs the real GDT through the assembler function
void gdt_install()
{
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (unsigned int)&gdt;
  
  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
  
  gdt_flush();
}

// Declare the page directory and a page table, both 4kb-aligned
unsigned long kernelpagedir[1024] __attribute__ ((aligned (4096)));
unsigned long lowpagetable[1024] __attribute__ ((aligned (4096)));
 
// This function fills the page directory and the page table,
// then enables paging by putting the address of the page directory
// into the CR3 register and setting the 31st bit into the CR0 one
void init_paging()
{
	// Pointers to the page directory and the page table
	void *kernelpagedirPtr = 0;
	void *lowpagetablePtr = 0;
	int k = 0;
 
	kernelpagedirPtr = (char *)kernelpagedir + 0x40000000;	// Translate the page directory from
								// virtual address to physical address
	lowpagetablePtr = (char *)lowpagetable + 0x40000000;	// Same for the page table
        
	// Counts from 0 to 1023 to...
	for (k = 0; k < 1024; k++)
	{
          lowpagetable[k] = (k * 4096) | 0x3;	// ...map the first 4MB of memory into the page table...
          kernelpagedir[k] = 0;			// ...and clear the page directory entries
	}
 
	// Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
	// with the same page table
 
	kernelpagedir[0] = (unsigned long)lowpagetablePtr | 0x3;
	kernelpagedir[768] = (unsigned long)lowpagetablePtr | 0x3;
        
	// Copies the address of the page directory into the CR3 register and, finally, enables paging!
 
	asm volatile (	"mov %0, %%eax\n"
			"mov %%eax, %%cr3\n"
			"mov %%cr0, %%eax\n"
			"orl $0x80000000, %%eax\n"
			"mov %%eax, %%cr0\n" :: "m" (kernelpagedirPtr));
}
 
// Extern functions
void gdt_install();
void init_paging();
 
// Clears the screen
void cls()
{
	int i = 0;
 
	for (i = 0; i < 80 * 25; i++)
		video[i] = (attrib << 8) | 0;
}
 
// Prints the welcome message ;)
void helloworld()
{
	char msg[] = "Hello, World!";
	int i = 0;
 
	for (i = 0; msg[i] != '\0'; i++)
          video[i] = (attrib << 8) | msg[i];
}
 
// Our kernel's first function: kmain
void kmain()
{
  extern multiboot_uint32_t magic;
  char buf[20];

   // FIRST enable paging and THEN load the real GDT!
  init_paging();
  gdt_install();
  
  cls();
  itoa(buf, 'x', magic);
  puts(buf);
  puts("\n");
  puts("What\n");
 
  if( magic != (0x2BADB002)) {
    puts("Error");
    return;
  }
 
  // Hang up the computer
  for (;;);
}
