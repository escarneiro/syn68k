#include "alloc.h"
#include "destroyblock.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#ifdef HAVE_MMAP
#include <sys/mman.h>
#include <unistd.h>
#endif

/* Native code lives in memory obtained from malloc, which modern kernels
 * do not make executable.  Grant that permission to the pages that hold
 * [start, start + size).  No-op on hosts without mprotect.
 */
void
make_executable (void *start, size_t size)
{
#ifdef HAVE_MMAP
  unsigned long page_mask = ~((unsigned long) getpagesize () - 1);
  unsigned long first = (unsigned long) start & page_mask;
  unsigned long last = ((unsigned long) start + size - 1) & page_mask;

  if (mprotect ((void *) first, last - first + getpagesize (),
		PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    {
      perror ("mprotect of native code");
      exit (-1);
    }
#endif
}

void *
xmalloc (size_t size)
{
  void *p;

  while ((p = malloc (size)) == NULL)
    {
      if (!destroy_any_block ())
	{
	  fprintf (stderr, "Out of memory.  Tried to alloc 0x%lX bytes.\n",
		   (unsigned long) size);
	  exit (-1);
	}
    }

  return p;
}


void *
xrealloc (void *old, size_t new_size)
{
  void *p;

  while ((p = realloc (old, new_size)) == NULL && new_size)
    {
      if (!destroy_any_block ())
	{
	  fprintf (stderr, "Out of memory.  Tried to realloc 0x%lX bytes.\n",
		   (unsigned long) new_size);
	  exit (-1);
	}
    }

  return p;
}


void *
xcalloc (size_t num_elems, size_t byte_size)
{
  void *p;
  
  while ((p = calloc (num_elems, byte_size)) == NULL)
    {
      if (!destroy_any_block ())
	{
	  fprintf (stderr, "Out of memory.  Tried to calloc 0x%lX bytes.\n",
		   (unsigned long) (num_elems * byte_size));
	  exit (-1);
	}
    }

  return p;
}
