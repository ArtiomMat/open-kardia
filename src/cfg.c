#include "cfg.h"

// Honeslty no clue, not even gonna question it, got it from IBM
#define _OPEN_SYS_ITOA_EXT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// An implicit limit so that we don't destroy the RAM if the dev opens the wrong file.
#define VAR_MAX_NAME 32
// To be able to contain a full blown unisnged long long
#define VAR_MAX_S 32

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
    unsigned long long u;
    long long i;
    char s[VAR_MAX_S];
  };
  short flags;
  short type;
} var_t;

static var_t* vars;
static int vars_n;

void
cfg_sets(cfg_id_t id, const char* str)
{
  strncpy(vars[id].s, str, VAR_MAX_S);
  vars[id].type = TYPE_S;
}

void
cfg_seti(cfg_id_t id, long long x)
{
  vars[id].i = x;
  vars[id].type = TYPE_I;
}

extern void
cfg_setu(cfg_id_t id, unsigned long long u)
{
  vars[id].u = u;
  vars[id].type = TYPE_U;
}

char*
cfg_gets(cfg_id_t id)
{
  static char s[VAR_MAX_S];
  if (vars[id].type == TYPE_I)
  {
    sprintf(s, "%lli", vars[id].i);
  }

  return vars[id].s;
}
long long
cfg_geti(cfg_id_t id)
{
  if (vars[id].type == TYPE_S)
  {
    return atoll(vars[id].s);
  }

  return vars[id].i;
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

  // Setup vars_n
  var_t v = {0}; // Temporary variable for syntax validation
  int word = 0, i = 0; // Word index, i is relative to the word's start
  int vn = 0, vd = 0; // name and data indices of v respectively.
  vars_n = 1;
  int c;
  while ((c = fgetc(f)) != EOF)
  {
    if (word == 0)
    {
      if (i == 0 && c == '[')
      {

      }
    }
    if (c == '\n')
    {
      word = i = 0;
      vars_n++;
    }
    if (vars_n > CFG_VAR_LIMIT)
    {
      fprintf(stderr, "cfg_init(): '%s' may be either invalid, or has too many variables.", fp);
      return 0;
    }
  }

  // Setup vars
  rewind(f);
  vars = malloc(sizeof (*vars) * vars_n);

  return 0; // TODO: Make it 1
}

