// A program for compiling GUI text code

#include "../engine/gui.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>

#define LINE_MAX 1024

#define ROWS_MAX 32
#define COLS_MAX 32

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

static line_t* lines = NULL;

typedef struct thing
{
  int type;
  // id[0]=0 means that the thing has no id and cannot be identified(NOT THAT IT IS AN)
  char* id;
  char* str; // Allocated to fit the string

  int x, y, w, h, max_w, min_w, max_h, min_h;
  union
  {
    struct
    {
      char format;
    } itext;
    struct
    {
      int* cols_n; // rows_n elements here, provides the number of columns per given row index
      int* children; // Flattened array of all child indices
      int rows_n;
      int row_i, child_i;
    } rowmap;
    struct
    {
      int child; // Index of child
    } window;
  };
} thing_t;

static thing_t* things;
static int things_n = 0;

// Buffer for aiding with parsing, mainly used for extracting words/strings in the file.
// Not thread safe, shouldn't be an issue here, but just noting, it's used by functions like extract_int.
// static char aidbuf[LINE_MAX];

// Used for error printing by global functions like extract_ functions.
static int current_line = 0;

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
wrdcpy(char* restrict dst, const char* restrict src, int n, char c)
{
  for (int i = 0; i < n; i++)
  {
    // If we have the last ' then we good
    if (src[i] == c || src[i] == 0)
    {
      dst[i] = 0;
      return 1;
    }
    dst[i] = src[i];
  }
  return 0;
}

// Extracts a word by malloc()-ing it into *out.
// Returns the number of characters written(not including null terminator), essentially same output as wrdlen(src, c).
// Returns 0 if there was an issue or there is literally no word.
static int
extract_wrd(const char* restrict src, char* restrict* out, char c)
{
  int len = wrdlen(src, c);
  if (len)
  {
    int n = len + 1;
    *out = malloc(n);
    wrdcpy(*out, src, n, c);
    return len;
  }
  fprintf(stderr, "Line %d: Expected word but found nothing.\n", current_line);
  return 0;
}

// Test if str is an integer(includes negative).
// The string must be and integer until the null terminator, C is the seperator.
// Returns 0 if it isn't, otherwise the index of C or null terminator.
static int
test_wrd_int(const char* str, int c)
{
  int i = 0;
  // Skip spaces, if we reach *str=0 we will detect it no worries
  for (; str[i] == c; i++)
  {}

  if (str[i] == '-')
  {
    i++;
  }
  
  if (str[i] == 0)
  {
    return 0;
  }

  for (; str[i] && (str[i] != c); i++)
  {
    if (str[i] < '0' || str[i] > '9')
    {
      return 0;
    }
  }
  return i;
}

// Expects src to be an integer until a space or a null terminator
// Returns 0 if there was an error parsing the integer, otherwise test_wrd_int(src) which would be non 0 for the valid integer.
static int
extract_int(const char* src, int* i, char c)
{
  int len = test_wrd_int(src, c);
  if (len > 0)
  {
    *i = atoi(src);
    return len;
  }
  
  fprintf(stderr, "Line %d: Expected integer not '%s'.\n", current_line, src);
  return 0;
}

// n is the size of dst as an entire buffer.
// Copies a string that is in "'...'\0" format as "...\0".
// Returns 0 if did not succeed!
static int
guistrcpy(char* restrict dst, const char* restrict src, int n)
{
  // Skip first
  if (src[0] == '\'')
  {
    src++;
  }
  
  for (int i = 0, j = 0; src[i] && i < n; i++, j++)
  {
    // If we have the last ' then we good
    if (src[i] == '\\')
    {
      i++;
      if (src[i] == 'n')
      {
        dst[j] = '\n';
      }
      else if (src[i] == 't')
      {
        dst[j] = '\t';
      }
      else if (src[i] == '\\')
      {
        dst[j] = '\\';
      }
      else if (src[i] != '\'') // Not a character that just avoids confusion
      {
        dst[j++] = '\\';
        dst[j] = src[i];
        fprintf(stderr, "Line %d: Unknown combination of \\ and code, '\\%c', note that it is ignored.\n", current_line, src[i]);
      }
    }
    else if (src[i] == '\'' && src[i+1] == 0)
    {
      dst[i] = 0;
      return 1;
    }
    else
    {
      dst[j] = src[i];
    }
  }

  // If we get to N or src ends incorrectly we break and return 0
  return 0;
}

// Returns the exact* (N - 1) needed in guistrcpy(), N-1 because it returns length rather than general size with the null terminator.
// * - Not actually exact :) because it ignores \\ unlike strcpy which actually parses them.
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

// Extracts a guistr by malloc()-ing it.
// Returns 0 if the string is invalid, othersie guistrlen(src)
static int
extract_guistr(const char* restrict src, char* restrict* out)
{
  int len = guistrlen(src);
  
  if (len > 0)
  {
    int n = len + 1;
    *out = malloc(n);
    
    guistrcpy(*out, src, n);

    return len;
  }

  fprintf(stderr, "Line %d: String required, not '%s'.\n", current_line, src);
  return 0;
}

// asserted version
static int
a_extract_int(const char* src, int* i, char c)
{
  int ret = extract_int(src, i, c);
  if (!ret)
  {
    exit(1);
  }
  return ret;
}
// asserted version
static int
a_extract_guistr(const char *restrict src, char *restrict *out)
{
  int ret = extract_guistr(src, out);
  if (!ret)
  {
    exit(1);
  }
  return ret;
}
// asserted version
static int
a_extract_wrd(const char *restrict src, char *restrict *out, char c)
{
  int ret = extract_wrd(src, out, c);
  if (!ret)
  {
    exit(1);
  }
  return ret;
}

// A thing_i is searching for another thing with the ID.
// Returns -1 if not found.
static int
find_by_id(int thing_i, const char* id)
{
  for (int i = 0; i < things_n; i++)
  {
    if (i != thing_i && !strcmp(id, things[i].id))
    {
      return i;
    }
  }

  fprintf(stderr, "Line %d: Thing with ID '%s' does not exist.\n", current_line, id);
  return -1;
}

// asserted version
static int
a_find_by_id(int thing_i, const char* id)
{
  int ret = find_by_id(thing_i, id);
  if (ret == -1)
  {
    exit(1);
  }

  return ret;
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

    // Still going, if c=';' we have a comment
    if (c != '\n' && c != EOF)
    {
      if (!in_string)
      {
        // If it's the first space after the word don't skip, otherwise skip
        if (c==' ' || c=='\t')
        {
          // Inside to eat the else downtown
          if (!had_space)
          {
            line->str[i++] = ' ';
            had_space = 1;
          }
        }
        else if (c=='\'')
        {
          line->str[i++] = '\'';
          in_string = 1;
        }
        // A comment!
        else if (c=='#')
        {
          while (c != '\n' && c != EOF)
          {
            c = getc(in);
          }
          goto _finish_up_line;
        }
        else
        {
          line->str[i++] = c;
          had_space = 0;
        }
      }
      else // (in_string)
      {
        // Only if a there was an even number of slashes can we exit the string
        if (c == '\'' && slash_depth % 2 == 0)
        {
          in_string = 0;
        }
        else if (c == '\\')
        {
          slash_depth++;
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
      _finish_up_line:

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
        printf("[%s]\n", line->str);

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

  // Locate now all the t commands and just setup for now the IDs, type and reset the values of all the things to defaults.
  int thing_i = -1;
  for (line_t* l = lines; l != NULL; l = l->next)
  {
    char* str = l->str;
    current_line = l->real;

    if (str[0] == 't' && str[1] == ' ')
    {
      thing_i++;

      str += 2;
      int end;
      if ((end = wrdcmp("window", str, ' ')))
      {
        things[thing_i].type = GUI_T_WINDOW;
      }
      else if ((end = wrdcmp("rowmap", str, ' ')))
      {
        things[thing_i].type = GUI_T_ROWMAP;

        int saved_line = current_line; // To be recovered later

        int rows_n = 0;
        int cols_n[COLS_MAX] = {0};
        int total_cols_n = 0;

        // Start looking up the row numbers
        for (line_t* l2 = l->next; l2 != NULL; l2 = l2->next)
        {
          char* str2 = l2->str;
          current_line = l2->real;
          // That's it we got what we got
          if (str2[0] == 't' && str2[1] == ' ')
          {
            break;
          }
          else if (wrdcmp("row", str2, ' '))
          {
            rows_n++;
            if (rows_n > ROWS_MAX)
            {
              fprintf(stderr, "Line %d: Rows exceeded %d limit.\n", current_line, ROWS_MAX);
              return 1;
            }
          }
          else if (wrdcmp("col", str2, ' '))
          {
            if (!rows_n)
            {
              fprintf(stderr, "Line %d: Putting columns before using the row command.\n", current_line);
              return 1;
            }
            
            cols_n[rows_n-1]++;
            total_cols_n++;
          }
        }

        if (!rows_n)
        {
          fprintf(stderr, "Line %d: A rowmap with no rows?\n", current_line);
          return 1;
        }

        // Then copy over
        things[thing_i].rowmap.rows_n = rows_n;
        things[thing_i].rowmap.cols_n = malloc(sizeof(*things[thing_i].rowmap.cols_n) * rows_n);
        for (int i = 0; i < rows_n; i++)
        {
          things[thing_i].rowmap.cols_n[i] = cols_n[i];
        }

        // Also allocate the children
        things[thing_i].rowmap.children = malloc(sizeof(*things[thing_i].rowmap.children) * total_cols_n);

        current_line = saved_line;

        // Also reset stuff
        things[thing_i].rowmap.child_i = 0;
        things[thing_i].rowmap.row_i = -1;
      }
      else if ((end = wrdcmp("button", str, ' ')))
      {
        things[thing_i].type = GUI_T_BUTTON;
      }
      else if ((end = wrdcmp("itext", str, ' ')))
      {
        things[thing_i].type = GUI_T_ITEXT;

        things[thing_i].itext.format = 0;
      }
      else if ((end = wrdcmp("otext", str, ' ')))
      {
        things[thing_i].type = GUI_T_OTEXT;
      }
      else
      {
        str[ wrdlen(str, ' ') ] = 0;

        fprintf(stderr, "Line %d: Unknown thing type used '%s'.\n", current_line, str);
        return 1;
      }

      str += end;

      if (str[0] == 0) // Meaning the line ended and there is no ID
      {
        fprintf(stderr, "Line %d: Thing must have an ID.\n", current_line);
        return 1;
      }

      // Now just copy the stuff
      str++;
      a_extract_wrd(str, &things[thing_i].id, ' ');
      // puts(things[thing_i].id);

      // Setup defaults
      things[thing_i].str = "NULL";
      things[thing_i].x = 0;
      things[thing_i].y = 0;
      things[thing_i].max_w = 100000;
      things[thing_i].max_h = 100000;
      things[thing_i].min_w = 1;
      things[thing_i].min_h = 1;
    }
    // We want to count up the rows and shit
    else if (things[thing_i].type == GUI_T_ROWMAP)
    {
      
    }
  }
  
  // Now we actually setup all the things
  thing_i = -1;
  for (line_t* l = lines; l != NULL; l = l->next)
  {
    char* str = l->str;
    current_line = l->real;
    // This time we just increment
    if (str[0] == 't' && str[1] == ' ')
    {
      thing_i++;
    }
    // PARSING ALL THE OTHER PARAMETERS ***IF*** WE ARE INSIDE A THING
    else if (thing_i >= 0)
    {
      int end;

      // General commands
      if ((end = wrdcmp("str", str, ' ')))
      {
        str += end + 1;
        a_extract_guistr(str, &things[thing_i].str);
      }
      else if ((end = wrdcmp("x", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].x, ' ');
      }
      else if ((end = wrdcmp("y", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].y, ' ');
      }
      else if ((end = wrdcmp("w", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].w, ' ');
      }
      else if ((end = wrdcmp("min_w", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].min_w, ' ');
      }
      else if ((end = wrdcmp("max_w", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].max_w, ' ');
      }
      else if ((end = wrdcmp("h", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].h, ' ');
      }
      else if ((end = wrdcmp("min_h", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].min_h, ' ');
      }
      else if ((end = wrdcmp("max_h", str, ' ')))
      {
        str += end + 1;
        a_extract_int(str, &things[thing_i].max_h, ' ');
      }

      // WINDOW specific commands
      else if (things[thing_i].type == GUI_T_WINDOW && (end = wrdcmp("child", str, ' ')))
      {
        str += end + 1;
        things[thing_i].window.child = a_find_by_id(thing_i, str);
      }

      // ROWMAP specific commands
      else if (things[thing_i].type == GUI_T_ROWMAP && (end = wrdcmp("row", str, ' ')))
      {
        things[thing_i].rowmap.row_i++; // Starts at -1
      }
      else if (things[thing_i].type == GUI_T_ROWMAP && (end = wrdcmp("col", str, ' ')))
      {
        str += end + 1;

        things[thing_i].rowmap.children
        [
          things[thing_i].rowmap.child_i
        ] = a_find_by_id(thing_i, str);

        // puts(things[things[thing_i].rowmap.children[things[thing_i].rowmap.child_i]].id);

        things[thing_i].rowmap.child_i++; // Starts at 0
      }

      // ITEXT specific commands
      else if (things[thing_i].type == GUI_T_ITEXT && (end = wrdcmp("format", str, ' ')))
      {
        str += end + 1;

        char* format;
        int flags = GUI_ITXT_NONUM | GUI_ITXT_NOALP | GUI_ITXT_NOSYM | GUI_ITXT_NOSPC; // The flags (GUI_ITXT_*)
        a_extract_guistr(str, &format);

        for (int i = 0; format[i]; i++)
        {
          switch (format[i])
          {
            case 'n':
            case 'N':
            flags &= ~GUI_ITXT_NONUM;
            break;
            
            case 'a':
            case 'A':
            flags &= ~GUI_ITXT_NOALP;
            break;
            
            case 's':
            case 'S':
            flags &= ~GUI_ITXT_NOSYM;
            break;
            
            case ' ':
            case '\t':
            flags &= ~GUI_ITXT_NOSPC;
            break;
            
            case '*':
            case 'x':
            case 'X':
            flags &= GUI_ITXT_PASSWORD;
            break;

            default:
            fprintf(stderr, "Line %d: Unknown format option '%c'.\n", current_line, format[i]);
            return 1;
          }
        }

        things[thing_i].itext.format = flags;

        free(format);
      }

      // Error!
      else
      {
        str[ wrdlen(str, ' ') ] = 0;

        fprintf(stderr, "Line %d: Unknown command '%s'.\n", current_line, str);
        return 1;
      }
    }
    else
    {
      fprintf(stderr, "Line %d: Junk.\n", current_line);
    }
  }
  
  fclose(in);
  fclose(out);
  return 0;
}
