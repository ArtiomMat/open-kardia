#include "gui.h"

#include <stdio.h>

// Parser for .gui files

enum
{
 _
};

// Compare until c is met, or null terminator.
// Returns 0 if equal, not 0 if not, to work like strcmp()
extern int
strcmpc(const char* a, const char* b, char c)
{
  while (*a == *b)
  {
    if (!(*a) || *a == c) // Then *b=c
    {
      return 0;
    }
    a++;
    b++;
  }
  return 1;
}

int
gui_open(const char* fp)
{
  if (gui_things_n)
  {
    gui_free();
  }

  FILE* f = fopen(fp, "rb");

  // Testing the type
  int is_bin = 0;
  {
    char first[3];
    if (!fread(first, 1, 3, f))
    {
      goto _fail_read;
    }

    // "# \n"
    if (first[0] != '#' || first[2] != '\n')
    {
      goto _bad_read;
    }

    // "B" or "T"
    if (first[1] == 'B')
    {
      is_bin = 1;
    }
    else if (first[1] != 'T')// Not text!
    {
      goto _bad_read;
    }
  }

  if (!is_bin)
  {

  }
  else
  {

  }

  return 1;

  _bad_read:
  fprintf(stderr, "gui_open(): Bad type '%s', either '#T\\n' or '#B\\n', if using text make sure there are no trailing spaces.", fp);
  _fail_read:
  fprintf(stderr, "gui_open(): Failed to read '%s'.", fp);
  return 0;
}

