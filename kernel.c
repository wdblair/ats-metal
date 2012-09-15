#include <stdint.h>

void kmain(void)
{
  extern uint32_t magic;
  int i;
  //extern void *mbd;
  
  if ( magic != 0x2BADB002 )
   {
     /* Something went not according to specs. Print an error */
     /* message and halt, but do *not* rely on the multiboot */
     /* data structure. */
   }
  
  /* You could either use multiboot.h */
  /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
  /* or do your offsets yourself. The following is merely an example. */ 
  //char * boot_loader_name =(char*) ((long*)mbd)[16];
  
  /* Print a letter to screen to see everything is working: */
  unsigned char *videoram = (unsigned char *)0xB8000;
  
  //clear the screen
  for(i = 1; i < 2000; i += 2) {
    videoram[i] = 0;
  }

  videoram[0] = 65; /* character 'A' */
  videoram[1] = 0x2a; /* light grey (7) on black (0). */
}
