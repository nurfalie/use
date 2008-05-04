#ifndef _FLAGS_H_
#define _FLAGS_H_

/*
** -- Defines --
*/

#define MAX_PRODUCTS 50
#define MAX_PRODUCT_NAME_LENGTH 128

struct flags_struct
{
  int shell_type;
  int pretend;
  int no_path;
  int no_manpath;
  int no_ld_library_path;
  int no_xfilesearchpath;
  int used_found[MAX_PRODUCTS];
  int detached_found[MAX_PRODUCTS];
  char used[MAX_PRODUCTS][MAX_PRODUCT_NAME_LENGTH];
  char detached[MAX_PRODUCTS][MAX_PRODUCT_NAME_LENGTH];
  short list;
  short about;
  short quiet;
  short items_used;
  short items_detached;
};

#endif
