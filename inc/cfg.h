// Configuration module, works as a realtime configuration editor

#define CFG_MAX_VARS 32

extern void
cfg_init(const char* fp);

extern void
cfg_free();

/**
 * This is not the fastest, O(N) in worst case scenario.
 * One time per each variable, once it's found you can't refind it. to speed this up as much as I can.
 * Returns an ID for the variable.
 */
extern int
cfg_find(const char* name);

/**
 * STR is copied over.
 */
extern void
cfg_sets(int id, const char* str);

extern void
cfg_seti(int id, long long x);

extern void
cfg_setu(int id, unsigned long long u);

