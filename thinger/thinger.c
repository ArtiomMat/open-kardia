// A program for compiling GUI text code

#include <stdio.h>

#include "gui.h"
#include "com.h"

int main(int args_n, const char** args)
{
  if (args_n < 2)
  {
    fprintf(stderr, "Usage: %s <input file> [output file]\n", args[0]);
    return 1;
  }
  
  char* outfp;
  if (args_n < 3)
  {
    puts("TODO");
    return 1;
  }
  else
  {
    outfp = args[2];
  }
  
  FILE* in = fopen(args[1], "rb");
  if (in == NULL)
  {
    fprintf(stderr, "%s does not exist.\n", args[1]);
    return 1;
  }
  
  // Output check if exists
  FILE* out = fopen(outfp, "r");
  if (out != NULL)
  {
    fclose(out);
    
    printf("%s already exists, override it? [Y/n] ", outfp);
    fflush(stdout);
    
    int c = fgetc(stdin);
    if (c != 'Y')
    {
      puts("Exiting.");
      return 0;
    }
  }
  
  // Reopen or open output for writing
  out = fopen(outfp, "wb");
  
  
  
  fclose(in);
  fclose(out);
  return 0;
}

