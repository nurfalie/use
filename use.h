#ifndef _USE_H_
#define _USE_H_

/*
** -- System Includes --
*/

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>

/*
** -- Local Includes --
*/

#include "flags.h"
#include "use_tmp.h"

char filename[PATH_MAX];
FILE *_stdout_;

/*
** -- Defines --
*/

#define VERSION "1.10"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define MAX_LINE_LENGTH (PATH_MAX + MAX_PRODUCT_NAME_LENGTH + 128)

#define ADD_PATH 0
#define DELETE_PATH 1

enum
  {
    SH = 1,
    CSH,
    KSH,
    BASH,
    TCSH
  };

/*
** -- Local Functions --
*/

int validate(const int, char *[], struct flags_struct *);
int validatePath(const char *, const struct flags_struct *);

#endif
