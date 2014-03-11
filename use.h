#ifndef _USE_H_
#define _USE_H_

/*
** -- System Includes --
*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

/*
** -- Local Includes --
*/

#include "flags.h"
#include "use_tmp.h"

char filename[PATH_MAX];
FILE *_stdout_;

#define VERSION "1.10.9"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#define MAX_LINE_LENGTH (PATH_MAX + MAX_PRODUCT_NAME_LENGTH + 128)

#define ADD_PATH 0
#define DELETE_PATH 1

enum
  {
    BASH = 1,
    CSH,
    KSH,
    SH,
    TCSH
  };

int validate(const int argc, char *argv[], struct flags_struct *flags);
int validatePath(const char *path, const struct flags_struct *flags);

#endif
