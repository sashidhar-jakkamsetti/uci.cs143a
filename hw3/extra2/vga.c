/* Taken help from online resources to write and modify this file*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>


#define NCOL 80
#define NROW 66

static unsigned int cx, cy;

#define CYAN 0x03

static unsigned char (*disp)[NCOL][2];

static void clear_row (unsigned int y); 
static void newline (void);
static void move_cursor (void);
static void locate_cursor (unsigned int *x, unsigned int *y);

static inline void
outw (unsigned short port, unsigned short data)
{
  asm volatile ("outw %0,%w1" : : "a" (data), "d" (port));
}

static inline void
outb (unsigned short port, unsigned char data)
{
  asm volatile ("outb %0,%w1" : : "a" (data), "d" (port));
}

static inline unsigned char
inb (unsigned short port)
{
  unsigned char data;
  asm volatile ("inb %w1,%0" : "=a" (data) : "d" (port));
  return data;
}


void
vgainit (void)
{
    disp = (void *)0xb8000;
    locate_cursor (&cx, &cy);
}

void
vga_putc (int c)
{
  switch (c) 
    {
    case '\n':
      newline ();
      break;

    default:
      disp[cy][cx][0] = c;
      disp[cy][cx][1] = CYAN;
      if (++cx >= NCOL)
        newline ();
      break;
    }

  move_cursor ();

}

void *
memmove (void *dst_, const void *src_, unsigned int size) 
{
  unsigned char *dst = dst_;
  const unsigned char *src = src_;

  if (dst < src) 
    {
      while (size-- > 0)
        *dst++ = *src++;
    }
  else 
    {
      dst += size;
      src += size;
      while (size-- > 0)
        *--dst = *--src;
    }

  return dst;
}


static void
clear_row (unsigned int y) 
{
  unsigned int x;

  for (x = 0; x < NCOL; x++)
    {
      disp[y][x][0] = ' ';
      disp[y][x][1] = CYAN
    ;
    }
}

static void
newline (void)
{
  cx = 0;
  cy++;
  if (cy >= NROW)
    {
      cy = NROW - 1;
      memmove (&disp[0], &disp[1], sizeof disp[0] * (NROW - 1));
      clear_row (NROW - 1);
    }
}

static void
move_cursor (void) 
{
  unsigned short cp = cx + NCOL * cy;
  outw (0x3d4, 0x0e | (cp & 0xff00));
  outw (0x3d4, 0x0f | (cp << 8));
}

static void
locate_cursor (unsigned int *x, unsigned int *y) 
{
  unsigned short cp;

  outb (0x3d4, 0x0e);
  cp = inb (0x3d5) << 8;

  outb (0x3d4, 0x0f);
  cp |= inb (0x3d5);

  *x = cp % NCOL;
  *y = cp / NCOL;
}


void
printvga(char *str)
{
    int i, c;

    for(i = 0; (c = str[i]) != 0; i++) {
        vga_putc(c);
    }
}
