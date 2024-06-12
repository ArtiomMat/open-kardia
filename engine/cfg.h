// Configuration module, works as a realtime configuration editor

#pragma once

#define CFG_VAR_LIMIT 128

// The values of the enum represent the shift not the flag itself.
enum
{
  CFG_F_NOSET = 1<<0, // The variable is readonly, value provided by file is the only one.
  CFG_F_PRIVATE = 1<<1, // The variable is only meant for the program to interact with, not for the user of the program. Useful for programs with a developer console.

  _CFG_F_FOUND = 1<<5, // For internal use!
};

extern int
cfg_init(const char* fp);

extern void
cfg_free();

/**
 * This is not the fastest, O(N) in worst case scenario.
 * One time per each variable, once it's found you can't refind it. to speed this up as much as I can.
 * Returns an ID for the variable, -1 if not found.
 */
extern int
cfg_find(const char* name);

/**
 * Test if a flag is set in a variable.
 */
extern int
cfg_flag(int id, int flag);

/**
 * STR is copied over.
 */
extern void
cfg_sets(int id, const char* str);
extern void
cfg_seti(int id, long long x);

// Not thread safe, uses a static char[]!
extern char*
cfg_gets(int id);
extern long long
cfg_geti(int id);

