#include "com.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char** com_args;
int com_args_n;

int _com_dir_end; // Where to splice the relative path in com_relfp

char com_dir[COM_PATH_SIZE];

// Initializes com_dir after the executable fp has been written to it
int
_com_init_dir()
{
  // Setting up _com_dir_end, and null terminating com_dir after the slash.
  int found_slash = 0;
  for (; _com_dir_end >= 0; _com_dir_end--)
  {
    if (com_dir[_com_dir_end] == '\\' || com_dir[_com_dir_end] == '/')
    {
      found_slash = 1;
      com_dir[++_com_dir_end] = 0;
      break;
    }
  }

  // If we didn't find a slash it means something went wrong, perhaps _com_dir_end was =-1 to begin with
  if (!found_slash)
  {
    fputs("com_init(): Getting executable's directory failed.", stderr);
    return 0;
  }

  puts(
    "com_init(): Common module initialized, "
    #ifdef COM_LILE
      "lil"
    #else
      "big"
    #endif
    " endian system.");

  return 1;
}

int
com_arg(const char* str)
{
  for (int i = 1; i < com_args_n; i++)
  {
    if (!strcmp(str, com_args[i]))
    {
      return i;
    }
  }
  return 0;
}

const char*
com_relfp(const char* p)
{
  int dir_i = _com_dir_end;
  for (int i = 0; p[i]; i++, dir_i++)
  {
    if (dir_i >= COM_PATH_SIZE-1) // Too long!
    {
      return NULL;
    }

    com_dir[dir_i] = p[i];
  }

  com_dir[dir_i] = 0;

  return com_dir;
}

// com_node_t*
// com_init_node(void* p)
// {
//   com_node_t* n = malloc(sizeof (com_node_t));
//   n->p = p;
//   n->l = NULL;
//   n->r = NULL;
//   return n;
// }

// com_node_t*
// com_leftest_node(com_node_t* n)
// {
//   while (n->l != NULL)
//   {
//     n = n->l;
//   }
//   return n;
// }

// com_node_t*
// com_rightest_node(com_node_t* n)
// {
//   while (n->r != NULL)
//   {
//     n = n->r;
//   }
//   return n;
// }

// com_node_t*
// com_push_node(com_node_t* l, com_node_t* n)
// {
//   if (l == NULL)
//   {
//     n->l = n->r = NULL;
//     return n;
//   }

//   n->l = l;
//   n->r = l->r;

//   l->r = n;

//   return l;
// }

// com_node_t*
// com_pull_node(com_node_t* n)
// {
//   if (n->l == n)
//   {
//     n->l->r = n->r;
//   }
//   if (n->r == n)
//   {
//     n->r->l = n->l;
//   }

//   com_node_t* l = n->l;

//   n->l = n->r = NULL;

//   return l;
// }

// com_node_t*
// com_free_node(com_node_t* n)
// {
//   com_node_t* ret = com_pull_node(n);
//   free(n);
//   return ret;
// }

// void
// com_free_nodes(com_node_t* n)
// {
//   if (n == NULL)
//   {
//     return;
//   }

//   for (com_node_t* x = n->r; x; x = x->r)
//   {
//     free(x);
//   }
//   for (com_node_t* x = n->l; x; x = x->l)
//   {
//     free(x);
//   }
// }
