// A program for compiling GUI text code

#include <stdio.h>

#include "gui.h"
#include "com.h"

#define LINE_MAX 1024

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

int main(int args_n, const char** args)
{
  if (args_n < 2)
  {
    fprintf(stderr, "Usage: %s <input file> [output file]\n", args[0]);
    return 1;
  }
  
  const char* outfp;
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
    
    /* TODO: Uncommentint c = fgetc(stdin);
    
    printf("%s already exists, override it? [Y/n] ", outfp);
    fflush(stdout);
    
    if (c != 'Y')
    {
      puts("Exiting.");
      return 0;
    }*/
  }
  
  // Reopen or open output for writing
  out = fopen(outfp, "wb");
  
  char line[LINE_MAX];
  int lines_n = 1; // How many lines we got
  int c;

  int i = 0; // The index of the character withing the line
  int had_space = 1; // Did we have a space last character?
  int in_string = 0; // Are we inside of a string rn? used to avoid removal of extra spaces if it's indeed a string.

  while ((c = getc(in)))
  {
    // Make sure i is within range
    if (i >= LINE_MAX)
    {
      fprintf(stderr, "cfg_init(): Line %d is impractically long.\n", lines_n);
      return 1;
      
      #if 0
      // Skip until we either hit EOF or until it's newline, to allow to just recover gracefully from the error
      while ((c = getc(f)) != EOF)
      {
        if (c == '\n')
        {
          break;
        }
      }

      continue;
      #endif
    }
    
    
    // Still going
    if (c != '\n' && c != EOF)
    {
      
      // So if we are outside a string, and there are extra spaces, just skip them.
      if (!in_string && (c==' ' || c=='\t'))
      {
        if (!had_space) // Don't skip if first space
        {
          line[i++] = ' ';
          had_space = 1;
        }
      }
      else
      {
        if (c == '\'')
        {
          // TODO: We also need slash_depth variable for how many slashes were before the ' to detect if we aren't actually wanting to close it up. And if it's odd then we don't want to close it yet \\\ means we don't \\ means we do, ykwis?
          in_string = !in_string; // Just swap it. works.
        }
        
        had_space = 0;
        line[i++] = c;
      }
      
    }
    // We are done with this line, parse it now.
    else
    {
      // We may have finished with a trailing space, so remove it
      if (i > 1 && line[i-1] == ' ')
      {
        --i;
      }
      
      line[i] = 0;
      
      if (i > 0)
      {
        printf("[%s]\n", line);
      }
      
      // That was the last line.
      if (c == EOF)
      {
        break;
      }
      
      // RESET EVERYTHING
      i = 0;
      had_space = 1;
    }
  }
  
  fclose(in);
  fclose(out);
  return 0;
}

