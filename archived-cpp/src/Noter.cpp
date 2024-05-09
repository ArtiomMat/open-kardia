#include "Noter.hpp"

#include <cstdio>
#include <cstdarg>

int Noter::levelFilter = NOTE_WHOTF;
Noter* Noter::currentNoter = nullptr;

Noter::Noter(const char* name)
{
  this->name = name;
}

const char levelStrings[][4] = {"USR", "DEV", "NRD", "???"};

void Noter::note(int level, const char* fmt, ...)
{
  va_list vlst;
  char str[1024];

  if (level >= levelFilter)
  {
    return;
  }

  // Are we interrupting another noter?
  if (currentNoter != nullptr && currentNoter != this)
  {
    printf("\n-- %s INTERRUPTING --\n", name, currentNoter->name);

    // Force last noter to have a new line, and also force us.
    currentNoter->nl = true;
    nl = true;
  }

  currentNoter = this;

  // If a new line we print the information
  if (nl)
  {
    printf("[%s#%s: ", levelStrings[level], name);
    nl = false;
  }

  va_start (vlst,fmt);
  int n = vsnprintf (str, sizeof (str) / sizeof (str[0]) , fmt ,vlst);
  va_end (vlst);

  // If we see a new line then next time we hit that nl and we are no longer noting
  if (str[n-1] == '\n')
  {
    str[n-1] = 0; // We print it ourselves with "]", watch below
    nl = true;
    currentNoter = nullptr;
  }

  fputs(str, stdout);
  if (currentNoter == nullptr) // Means that we just finished
  {
    fputs("]\n", stdout);
  }
  else
  {
    fflush(stdout);
  }
}
