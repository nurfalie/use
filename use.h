#ifndef _USE_H_
#define _USE_H_

#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "flags.h"
#include "use_tmp.h"

FILE *_stdout_;
FILE *_usefp_;
char TEMPDIR[PATH_MAX];
char filename[PATH_MAX];

#define VERSION "1.12.0"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#define ADD_PATH 0
#define DELETE_PATH 1
#define MAX_LINE_LENGTH (PATH_MAX + MAX_PRODUCT_NAME_LENGTH + 128)

enum
  {
    BASH = 1,
    CSH,
    KSH,
    SH,
    TCSH
  };

#endif
