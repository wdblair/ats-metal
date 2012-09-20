#include <stdint.h>
#include "multiboot.h"

static unsigned char *videoram = (unsigned char*)0xB8000;


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

#define bit_is_set(num, bit) (num & (1 << bit))

static unsigned int xpos, ypos;

#define LINES 25
#define COLUMNS 80
#define ATTRIBUTE 0x2a

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
     
  *(videoram + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
  *(videoram + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

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

void kmain(void)
{
  extern uint32_t magic;
  extern multiboot_info_t *mbd;
  int i;
  char buffer[32];
 
  if ( magic !=  MULTIBOOT_BOOTLOADER_MAGIC )
    {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
      puts("error\n");
      return;
    }
  
  /* You could either use multiboot.h */
  /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
  /* or do your offsets yourself. The following is merely an example. */ 
  //char * boot_loader_name =(char*) ((long*)mbd)[16];
  multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbd->mmap_addr;
  
  //clear the screen
  for(i = 1; i < 4000; i += 2) {
    videoram[i] = 0;
  }
  
  if (bit_is_set(mbd->flags, 6)) {
    
    while(mmap < mbd->mmap_addr + mbd->mmap_length) {
      mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int));
      
      if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
        puts("Memory Available!\n");
      } else
        continue;
      
      //Is ignoring the higher 32 bytes a problem??
      //      itoa(buffer, 'x', mmap->addr >> 32);
      //      puts(buffer);
      itoa(buffer, 'x', mmap->addr);
      puts(buffer);
      putchar(' ');
      //      itoa(buffer, 'x', mmap->len >> 32);
      //      puts(buffer);
      //The length of memory available.
      itoa(buffer, 'x', mmap->len);
      puts(buffer);
      putchar(' ');
      itoa(buffer, 'd', mmap->type);
      puts(buffer);
      putchar('\n');
    }
  }
  puts("Geschaft!");
  videoram[1] = 0x2a; /* light grey (7) on black (0). */
}
