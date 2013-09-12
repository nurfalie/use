#ifndef _FLAGS_H_
#define _FLAGS_H_

/*
** -- Defines --
*/

#define MAX_PRODUCTS 500
#define MAX_PRODUCT_NAME_LENGTH 128

struct flags_struct
{
  char detached[MAX_PRODUCTS][MAX_PRODUCT_NAME_LENGTH];
  char used[MAX_PRODUCTS][MAX_PRODUCT_NAME_LENGTH];
  int detached_found[MAX_PRODUCTS];
  int no_path;
  int no_manpath;
  int no_ld_library_path;
  int no_xfilesearchpath;
  int pretend;
  int shell_type;
  int used_found[MAX_PRODUCTS];
  short about;
  short items_used;
  short items_detached;
  short list;
  short quiet;
};

#endif
