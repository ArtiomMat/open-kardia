#include "cfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// An implicit limit so that we don't destroy the RAM if the dev opens the wrong file.
#define VAR_MAX_NAME 32
// To be able to contain a full blown unisnged long long
#define VAR_MAX_S 32

#define LINE_MAX 128

enum
{
  TYPE_S,
  TYPE_I,
  TYPE_U,
};

typedef struct
{
  char name[VAR_MAX_NAME];
  union
  {
    long long i;
    char s[VAR_MAX_S];
  };
  short flags;
  short type;
} var_t;

static var_t* vars;
static int vars_n;


extern int
cfg_find(const char* name)
{
  int id;

  for (id = 0; id < vars_n; id++)
  {
    if (vars[id].flags & _CFG_F_FOUND)
    {
      continue;
    }

    if (!strcmp(name, vars[id].name))
    {
      vars[id].flags |= _CFG_F_FOUND;
      return id;
    }
  }
  return -1;
}

void
cfg_sets(int id, const char* str)
{
  strncpy(vars[id].s, str, VAR_MAX_S);
  vars[id].type = TYPE_S;
}

void
cfg_seti(int id, long long x)
{
  vars[id].i = x;
  vars[id].type = TYPE_I;
}

char*
cfg_gets(int id)
{
  static char s[VAR_MAX_S];
  if (vars[id].type == TYPE_I)
  {
    sprintf(s, "%lli", vars[id].i);
  }

  return vars[id].s;
}
long long
cfg_geti(int id)
{
  if (vars[id].type == TYPE_S)
  {
    return atoll(vars[id].s);
  }

  return vars[id].i;
}

static void
parse_line()
{

}

int
cfg_init(const char* fp)
{
  FILE* f = fopen(fp, "r");
  if (f == NULL)
  {
    fprintf(stderr, "cfg_init(): '%s' does not exist.", fp);
    return 0;
  }

  char line[LINE_MAX];
  int lines_n = 1; // How many lines we got
  int c;

  // Setup vars_n and check syntax
  {
    int i = 0;

    while ((c = getc(f)) != EOF)
    {
      // Make sure i is within range
      if (i >= LINE_MAX)
      {
        fprintf(stderr, "cfg_init(): Line %d is impractically long, skipping it.\n", lines_n);

        // Skip until we either hit EOF or until it's newline, to allow to just recover gracefully from the error
        while ((c = getc(f)) != EOF)
        {
          if (c == '\n')
          {
            break;
          }
        }

        continue;
      }

      c = getc(f);

      // That's it we are done with the line
      if (c == '\n')
      {
        line[i] = 0;

      }
    }
  }

  // Setup vars
  rewind(f);
  vars = malloc(sizeof (*vars) * vars_n);
  lines_n = 1;

  fclose(f);
  return 1; // TODO: Make it 1
}

