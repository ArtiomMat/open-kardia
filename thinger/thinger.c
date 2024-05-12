// A program for compiling GUI text code

#include "../engine/gui.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>

#define LINE_MAX 1024

enum
{
  CWINDOW,
  CMAP,
  CITEXT,
  COTEXT,
  CBUTTON,
  CTICKBOX,
  CSLIDER,

  CX,
  CY,
  CH,
  CW,
  CWMIN,
  CWMAX,
  CHMIN,
  CHMAX,

  CTEXT,

  CFORMAT,
  CROW,
  CCHILD,

};

const char* cmdstrs[] =
{
  "window",
  "map",
  "itext",
  "otext",
  "button",
  "tickbox",
  "slider",

  "x",
  "y",
  "w",
  "wmin",
  "wmax",
  "h",
  "hmin",
  "hmax",

  "text",

  "format",
  "row",
  "child",
};

typedef struct line_s
{
  struct line_s* prev,* next;
  char str[LINE_MAX];
  int real; // The real line it's in(1 is the first not 0)
} line_t;

line_t* lines = NULL;

typedef struct thing
{
  int type;
  // id[0]=0 means that the thing has no id and cannot be identified(NOT THAT IT IS AN)
  char* id;
  char* str; // Allocated to fit the string

  int x, y, w, h, wmax, wmin, hmax, hmin;
  union
  {
    struct
    {
      char format;
    } itext;
    struct
    {
      char todo;
    } map;
    struct
    {
      char* child_id;
    } window;
  };
} thing_t;

thing_t* things;
int things_n = 0;

// Compare until C is met, or null terminator.
// Returns the index where the two words end(at C or null terminator) if found that a=b.
// Otherwise returns 0 if a!=b.
static int
wrdcmp(const char* a, const char* b, char c)
{
  int i = 0;
  while (*a == *b)
  {
    if (!(*a) || *a == c) // *b is C or 0 too
    {
      return i;
    }
    a++;
    b++;
    i++;
  }
  // Maybe the loop terminated because one ended early but the other too with another character
  if ((!(*a) && *b == c) || (!(*b) || *a == c))
  {
    return i;
  }
  return 0;
}

// Returns index where C was found or NULL terminator
static int
wrdlen(const char* a, char c)
{
  int i;
  for (i = 0; a[i] && a[i] != c; i++)
  {}
  return i;
}

// n is the size of the buffer dst.
// Returns 0 if did not succeed(src was too long).
static int
wrdcpy(char* dst, const char* src, int n, char c)
{
  for (int i = 0; i < n; i++)
  {
    // If we have the last ' then we good
    if (src[i] == c && src[i] == 0)
    {
      dst[i] = 0;
      return 1;
    }
    dst[i] = src[i];
  }
  return 0;
}

// Extracts a word by malloc()-ing it into *out.
// Returns the number of characters written(not including null terminator), essentially same output as wrdlen(src, c)
// Returns 0 if there was an issue or there is literally no word.
static int
extract_wrd(const char* src, char** out, char c)
{
  int len = wrdlen(src, c);
  if (len)
  {
    int n = len + 1;
    *out = malloc(n);
    wrdcpy(*out, src, n, c);
    return len;
  }
  return 0;
}

// n is the size of dst as an entire buffer.
// Copies a string that is in "'XXX'\0" format as "XXX\0".
// Returns 0 if did not succeed!
static int
guistrcpy(char* dst, const char* src, int n)
{
  // Skip first
  if (src[0] == '\'')
  {
    src++;
  }
  
  for (int i = 0; src[i] && i < n; i++)
  {
    // If we have the last ' then we good
    if (src[i] == '\'' && src[i+1] == 0)
    {
      dst[i] = 0;
      return 1;
    }
    dst[i] = src[i];
  }

  // If we get to N or src ends incorrectly we break and return 0
  return 0;
}

// Returns the exact (N - 1) needed in guistrcpy(), N-1 because it returns length rather than general size with the null terminator.
// Returns -1 if it's not a correct format.
static int
guistrlen(const char* src)
{
  // Skip first
  if (src[0] == '\'')
  {
    src++;
  }
  
  for (int i = 0; src[i]; i++)
  {
    // If we have the last ' then we good
    if (src[i] == '\'' && src[i+1] == 0)
    {
      return i;
    }
  }

  // If we get to N or src ends incorrectly we break and return 0
  return -1;
}

// Extracts a guistr by malloc()-ing it
// Returns NULL if the string is invalid
static char*
extract_guistr(const char* src)
{
  int len = guistrlen(src);
  
  if (len > 0)
  {
    int n = len + 1;
    char* str = malloc(n);
    
    guistrcpy(str, src, n);

    return str;
  }

  return NULL;
}

int
main(int args_n, const char** args)
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

  // char line[LINE_MAX];
  lines = malloc(sizeof (line_t));
  lines->next = NULL;
  lines->prev = NULL;

  // The current line
  line_t* line = lines;

  int lines_n = 1; // How many lines we got
  int c;

  int i = 0; // The index of the character withing the line
  int had_space = 1; // Did we have a space last character?
  int in_string = 0; // Are we inside of a string rn? used to avoid removal of extra spaces if it's indeed a string.
  int slash_depth = 0; // How many '\' in a row were detected, 0's when we see a character other than that

  while ((c = getc(in)))
  {
    // Make sure i is within range
    if (i >= LINE_MAX)
    {
      fprintf(stderr, "Line %d: Impractically long.\n", lines_n);
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
          line->str[i++] = ' ';
          had_space = 1;
        }
      }
      else
      {
        // Only if a there was an even number of slashes can we exit the string
        if (c == '\'' && slash_depth % 2 == 0)
        {
          in_string = !in_string;
        }
        else if (c == '\\')
        {
          slash_depth++; // Just swap it. works.
        }
        else
        {
          slash_depth = 0;
        }

        had_space = 0;
        line->str[i++] = c;
      }

    }
    // We are done with this line, parse it now.
    else
    {
      if (in_string) // Means that we did not end the string, the interpreter needs to be certain the last ' is the end.
      {
        fprintf(stderr, "Line %d: String did not terminate with '\n", lines_n);
        return 1;
      }

      // We may have finished with a trailing space, so remove it
      if (i > 1 && line->str[i-1] == ' ')
      {
        --i;
      }

      line->str[i] = 0;

      if (i > 0)
      {
        line->real = lines_n;
        line->next = malloc(sizeof (line_t));
      }
      else
      {
        line->next = NULL;
      }

      // That was the last line, so if there was a next we remove it
      if (c == EOF)
      {
        free(line->next);
        line->next = NULL;

        if (i == 0) // We need to also free it if it was empty
        {
          if (line->prev != NULL)
          {
            line->prev->next = NULL;
          }

          free(line);
        }

        break;
      }

      // If it's NULL it means i=0 so reuse the current line object
      if (line->next != NULL)
      {
        line->next->prev = line;
        line = line->next;
      }

      // RESET EVERYTHING
      i = 0;
      had_space = 1;
      lines_n++;
    }
  }

  // Count how many things we have!
  things_n = 0;
  for (line_t* l = lines; l != NULL; l = l->next)
  {
    if (l->str[0] == 't' && l->str[1] == ' ')
    {
      things_n++;
    }
  }

  things = malloc(sizeof(thing_t) * things_n);

  // Now we actually setup all the things
  int thing_i = -1;
  for (line_t* l = lines; l != NULL; l = l->next)
  {
    char* str = l->str;
    if (str[0] == 't' && str[1] == ' ')
    {
      thing_i++;

      str += 2;
      int end;
      if ((end = wrdcmp("window", str, ' ')))
      {
        things[thing_i].type = GUI_T_WINDOW;
      }
      else if ((end = wrdcmp("map", str, ' ')))
      {
        things[thing_i].type = GUI_T_MAP;
      }
      else if ((end = wrdcmp("button", str, ' ')))
      {
        things[thing_i].type = GUI_T_BUTTON;
      }
      else if ((end = wrdcmp("itext", str, ' ')))
      {
        things[thing_i].type = GUI_T_ITEXT;
      }
      else if ((end = wrdcmp("otext", str, ' ')))
      {
        things[thing_i].type = GUI_T_OTEXT;
      }
      else
      {
        str[ wrdlen(str, ' ') ] = 0;

        fprintf(stderr, "Line %d: Unknown type used '%s'.\n", l->real, str);
        return 1;
      }

      str += end;

      if (str[0] == 0) // Meaning the line ended and there is no ID
      {
        fprintf(stderr, "Line %d: Thing must have an ID.\n", l->real);
        return 1;
      }

      // Now just copy the stuff
      str++;
      extract_wrd(str, &things[thing_i].id, ' ');

      // Setup defaults
      things[thing_i].str = NULL;
      things[thing_i].x = 0;
      things[thing_i].y = 0;
      things[thing_i].wmax = 100000;
      things[thing_i].hmax = 100000;
      things[thing_i].hmin = 1;
      things[thing_i].wmin = 1;
    }
    // PARSING ALL THE OTHER PARAMETERS ***IF*** WE ARE INSIDE A THING
    else if (thing_i >= 0)
    {
      int end;
      if ((end = wrdcmp("str", str, ' ')))
      {
        str += end + 1;
        puts(extract_guistr(str));
      }
      else if ((end = wrdcmp("x", str, ' ')))
      {
        str += end;
      }
      else
      {
        str[ wrdlen(str, ' ') ] = 0;

        fprintf(stderr, "Line %d: Unknown command '%s'.\n", l->real, str);
        return 1;
      }
    }
  }
  
  fclose(in);
  fclose(out);
  return 0;
}
