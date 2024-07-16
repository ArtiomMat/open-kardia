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

typedef struct cfg_var_s
{
  struct cfg_var_s* prev, * next;
  char name[VAR_MAX_NAME];
  union
  {
    long long i;
    char s[VAR_MAX_S];
  };
  short flags;
  short type;
} var_t;

static var_t* last_var;
static int vars_n;

extern cfg_var_t
cfg_find(const char* name)
{
  int id;

  for (var_t* v = last_var; v != NULL; v = v->prev)
  {
    if (v->flags & _CFG_F_FOUND)
    {
      continue;
    }

    if (!strcmp(name, v->name))
    {
      v->flags |= _CFG_F_FOUND;
      return id;
    }
  }

  return NULL;
}

void
cfg_sets(cfg_var_t v, const char* str)
{
  strncpy(v->s, str, VAR_MAX_S);
  v->type = TYPE_S;
}

void
cfg_seti(cfg_var_t v, long long x)
{
  v->i = x;
  v->type = TYPE_I;
}

char*
cfg_gets(cfg_var_t v)
{
  static char s[VAR_MAX_S];
  if (v->type == TYPE_I)
  {
    sprintf(s, "%lli", v->i);
  }

  return v->s;
}
long long
cfg_geti(cfg_var_t v)
{
  if (v->type == TYPE_S)
  {
    return atoll(v->s);
  }

  return v->i;
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
        _skip_line:
        i = 0;
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
        i = 0;
        
        int j;
        
        for (j = 0; line[j] == ' ' || line[j] == '\t'; j++)
        {}

        if (line[j] == '#')
        {
          goto _skip_line;
        }

        
      }
    }
  }

  fclose(f);
  return 1; // TODO: Make it 1
}

