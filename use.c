/*
** Copyright (c) 2006, 2007 Alexis Megas
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "use.h"

/*
** -- Local variables --
*/

static char *PATH = NULL;
static char *MANPATH = NULL;
static char *LD_LIBRARY_PATH = NULL;
static char *XFILESEARCHPATH = NULL;

/*
** -- Local Functions --
*/

static int use(struct flags_struct *);
static int prepare(FILE *, const char *, const struct flags_struct *,
		   const int);
static int allocenv(char **, const char *, const int,
		    const struct flags_struct *);
static int updatevariable(const char *, const char *,
			  const struct flags_struct *, const int);
static void reset(struct flags_struct *);

int main(int argc, char *argv[])
{
  int rc = 0;
  char *tmp = NULL;
  char line[MAX_LINE_LENGTH];
  char filename[PATH_MAX];
  FILE *fp = NULL;
  struct flags_struct flags;

  (void) memset(filename, '\0', sizeof(filename));
  (void) snprintf(filename, sizeof(filename), "%s/.use.sourceme.%ld.%ld",
		  TEMPDIR, (long) getuid(), (long) getpid());

  if((_stdout_ = fopen(filename, "a+")) == NULL)
    {
      (void) fprintf(stdout, "/dev/null");
      return EXIT_FAILURE;
    }
  else
    (void) fprintf(stdout, "%s", filename);

  (void) setvbuf(_stdout_, (char *) NULL, _IONBF, 0);

  /*
  ** Reset the structure.
  */

  reset(&flags);

  if(validate(argc, argv, &flags) != 0)
    {
      /*
      ** Print the usage information.
      */

      rc = 1;

      if(!flags.quiet)
	(void) fprintf(_stdout_, "echo \"use: incorrect usage.\"\n");

      goto done_label;
    }

  if(flags.list)
    {
      if((fp = fopen(USETABLE, "r")) == NULL)
	{
	  rc = 1;
	  (void) fprintf(_stdout_, "echo \"Error: unable to open %s.\"",
			 USETABLE);
	}
      else
	{
	  while(fgets(line, (int) sizeof(line), fp) != NULL)
	    if(strstr(line, ".description") != NULL && strstr(line, ":")
	       != NULL && line[0] != '#')
	      {
		tmp = strstr(line, ":") + 1;

		if(tmp[strlen(tmp) - 1] == '\n')
		  tmp[strlen(tmp) - 1] = '\0';

		(void) fprintf(_stdout_, "echo \"%s\"\n", tmp);
	      }

	  (void) fclose(fp);
	}

      goto done_label;
    }
  else if(flags.about)
    {
      (void) fprintf(_stdout_, "echo \"USE\"\n");
      (void) fprintf(_stdout_, "echo \"Version: %s\"\n", VERSION);
      (void) fprintf(_stdout_, "echo \"Compilation Date & Time: %s %s\"\n",
		     __DATE__, __TIME__);
      (void) fprintf(_stdout_, "echo \"Use Table: %s\"\n", USETABLE);
      (void) fprintf(_stdout_, "echo \"Temporary Directory: %s\"\n", TEMPDIR);
      goto done_label;
    }

  rc = use(&flags);
  free(PATH);
  free(MANPATH);
  free(LD_LIBRARY_PATH);
  free(XFILESEARCHPATH);

 done_label:
  (void) fflush(_stdout_);
  (void) fclose(_stdout_);

  if(rc != 0)
    return EXIT_FAILURE;
  else
    return EXIT_SUCCESS;
}

static void reset(struct flags_struct *flags)
{
  int i = 0;

  flags->list = 0;
  flags->about = 0;
  flags->quiet = 0;
  flags->pretend = 0;
  flags->items_used = 0;
  flags->no_path = 0;
  flags->no_manpath = 0;
  flags->shell_type = 0;
  flags->items_detached = 0;
  flags->no_ld_library_path = 0;
  flags->no_xfilesearchpath = 0;

  for(i = 0; i < MAX_PRODUCTS; i++)
    {
      (void) memset(flags->used[i], '\0', sizeof(flags->used[0]));
      (void) memset(flags->detached[i], '\0', sizeof(flags->detached[0]));
    }
}

static int use(struct flags_struct *flags)
{
  int i = 0;
  int rc = 0;
  int found = 0;
  int dousevalues[4]; /* This should be the same size as stdvariables. */
  char *tmp = NULL;
  char line[MAX_LINE_LENGTH];
  char envact[MAX_LINE_LENGTH];
  char *product = NULL;
  char *stdvalues[4]; /* This should be the same size as stdvariables. */
  char *stdvariables[] = {"PATH",
			  "MANPATH",
			  "LD_LIBRARY_PATH",
			  "XFILESEARCHPATH"};
  size_t size = 0;
  FILE *fp = NULL;

  if(flags->no_path == 0)
    {
      if((tmp = getenv("PATH")) != NULL)
	size = strlen(tmp) + 1;
      else
	size = 1;

      PATH = (char *) malloc(size);
      (void) memset(PATH, '\0', size);

      if(PATH != NULL)
	{
	  if(tmp != NULL)
	    (void) strcpy(PATH, tmp);
	}
      else
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_, "echo \"Error: unable to allocate "
			   "memory for PATH.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "echo \"Warning: PATH not updated.\"\n");

  if(flags->no_manpath == 0)
    {
      if((tmp = getenv("MANPATH")) != NULL)
	size = strlen(tmp) + 1;
      else
	size = 1;

      MANPATH = (char *) malloc(size);
      (void) memset(MANPATH, '\0', size);

      if(MANPATH != NULL)
	{
	  if(tmp != NULL)
	    (void) strcpy(MANPATH, tmp);
	}
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_, "echo \"Error: unable to allocate "
			   "memory for MANPATH.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "echo \"Warning: MANPATH not updated.\"\n");

  if(flags->no_ld_library_path == 0)
    {
      if((tmp = getenv("LD_LIBRARY_PATH")) != NULL)
	size = strlen(tmp) + 1;
      else
	size = 1;

      LD_LIBRARY_PATH = (char *) malloc(size);
      (void) memset(LD_LIBRARY_PATH, '\0', size);

      if(LD_LIBRARY_PATH != NULL)
	{
	  if(tmp != NULL)
	    (void) strcpy(LD_LIBRARY_PATH, tmp);
	}
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_,
			   "echo \"Error: unable to allocate memory for "
			   "LD_LIBRARY_PATH.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "echo \"Warning: LD_LIBRARY_PATH not "
		   "updated.\"\n");

  if(flags->no_xfilesearchpath == 0)
    {
      if((tmp = getenv("XFILESEARCHPATH")) != NULL)
	size = strlen(tmp) + 1;
      else
	size = 1;

      XFILESEARCHPATH = (char *) malloc(size);
      (void) memset(XFILESEARCHPATH, '\0', size);

      if(XFILESEARCHPATH != NULL)
	{
	  if(tmp != NULL)
	  (void) strcpy(XFILESEARCHPATH, tmp);
	}
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_,
			   "echo \"Error: unable to allocate memory for "
			   "XFILESEARCHPATH.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "echo \"Warning: XFILESEARCHPATH not "
		   "updated.\"\n");

  if((fp = fopen(USETABLE, "r")) != NULL)
    {
      for(i = flags->items_detached - 1; i >= 0; i--)
	{
	  found = 0;

	  while(fgets(line, (int) sizeof(line), fp) != NULL)
	    if(strstr(line, ".description") != NULL && strstr(line, ":")
	       != NULL &&
	       line[0] != '#')
	      {
		product = line;

		if(strstr(product, ".") != NULL)
		  {
		    product[strlen(product) -
			    strlen(strstr(product, "."))] = '\0';

		    if(strcmp(flags->detached[i], product) == 0)
		      {
			found = 1;

			if(prepare(fp, product, flags, DELETE_PATH) != 0)
			  {
			    rc = 1;

			    if(!flags->quiet)
			      (void) fprintf
				(_stdout_,
				 "echo \"Error: unable to allocate "
				 "memory for product %s.\"\n",
				 product);

			    goto done_label;
			  }
			else if(!flags->quiet && !flags->pretend)
			  (void) fprintf(_stdout_, "echo \"%s has been "
					 "detached successfully.\"\n",
					 product);

			break;
		      }
		  }
		else
		  {
		    rc = 1;

		    if(!flags->quiet)
		      (void) fprintf(_stdout_,
				     "echo \"Error: possible "
				     "misconfiguration with %s.\"\n",
				     USETABLE);

		    break;
		  }
	      }

	  if(found != 1 && !flags->quiet)
	    (void) fprintf(_stdout_, "echo \"Warning: product %s "
			   "not found.\"\n",
			   flags->detached[i]);

	  rewind(fp);
	}

      for(i = flags->items_used - 1; i >= 0; i--)
	{
	  found = 0;

	  while(fgets(line, (int) sizeof(line), fp) != NULL)
	    if(strstr(line, ".description") != NULL && strstr(line, ":")
	       != NULL && line[0] != '#')
	      {
		product = line;

		if(strstr(product, ".") != NULL)
		  {
		    product[strlen(product) -
			    strlen(strstr(product, "."))] = '\0';

		    if(strcmp(flags->used[i], product) == 0)
		      {
			found = 1;

			if(prepare(fp, product, flags, ADD_PATH) != 0)
			  {
			    rc = 1;

			    if(!flags->quiet)
			      (void) fprintf
				(_stdout_,
				 "echo \"Error: unable to allocate "
				 "memory for product %s.\"\n",
				 product);

			    goto done_label;
			  }
			else if(!flags->quiet && !flags->pretend)
			  (void) fprintf(_stdout_, "echo \"%s has been "
					 "accessed successfully.\"\n",
					 product);

			break;
		      }
		  }
		else
		  {
		    rc = 1;

		    if(!flags->quiet)
		      (void) fprintf(_stdout_,
				     "echo \"Error: possible "
				     "misconfiguration with %s.\"\n",
				     USETABLE);

		    break;
		  }
	      }

	  if(found != 1 && !flags->quiet)
	    (void) fprintf(_stdout_, "echo \"Warning: product %s "
			   "not found.\"\n", flags->used[i]);

	  rewind(fp);
	}
    }
  else
    {
      rc = 1;

      if(!flags->quiet)
	(void) fprintf(_stdout_, "echo \"Error: unable to "
		       "open %s.\"\n", USETABLE);
    }

 done_label:

  if(fp != NULL)
    (void) fclose(fp);

  if(rc == 0)
    {
      /*
      ** Update the sourced file.
      */

      stdvalues[0] = PATH;
      stdvalues[1] = MANPATH;
      stdvalues[2] = LD_LIBRARY_PATH;
      stdvalues[3] = XFILESEARCHPATH;
      dousevalues[0] = flags->no_path;
      dousevalues[1] = flags->no_manpath;
      dousevalues[2] = flags->no_ld_library_path;
      dousevalues[3] = flags->no_xfilesearchpath;

      for(i = 0; i < 4; i++)
	if(stdvalues[i] != NULL &&
	   strlen(stdvalues[i]) > 0) /* In case the -n option was used. */
	  {
	    if(flags->shell_type == SH)
	      (void) snprintf(envact, sizeof(envact), "export %s=%s",
			      stdvariables[i],
			      stdvalues[i]);
	    else if(flags->shell_type == CSH)
	      (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			      stdvariables[i],
			      stdvalues[i]);
	    else if(flags->shell_type == KSH)
	      (void) snprintf(envact, sizeof(envact), "export %s=%s",
			      stdvariables[i],
			      stdvalues[i]);
	    else if(flags->shell_type == BASH)
	      (void) snprintf(envact, sizeof(envact), "export %s=%s",
			      stdvariables[i],
			      stdvalues[i]);
	    else if(flags->shell_type == TCSH)
	      (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			      stdvariables[i],
			      stdvalues[i]);

	    if(flags->pretend)
	      (void) fprintf(_stdout_, "echo \"%s\"\n", envact);
	    else
	      (void) fprintf(_stdout_, "%s\n", envact);
	  }
	else if(dousevalues[i] == 0)
	  {
	    if(flags->shell_type == SH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == CSH)
	      (void) snprintf(envact, sizeof(envact), "unsetenv %s",
			      stdvariables[i]);
	    else if(flags->shell_type == KSH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == BASH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == TCSH)
	      (void) snprintf(envact, sizeof(envact), "unsetenv %s",
			      stdvariables[i]);

	    if(flags->pretend)
	      (void) fprintf(_stdout_, "echo \"%s\"\n", envact);
	    else
	      (void) fprintf(_stdout_, "%s\n", envact);
	  }
    }

  return rc;
}

static int prepare(FILE *fp, const char *product,
		   const struct flags_struct *flags,
		   const int action)
{
  int rc = 0;
  char line[MAX_LINE_LENGTH];
  char *lasts;
  char *value = NULL;
  char *variable = NULL;

  /*
  ** The order of the products listed is significant. Products at the
  ** front of the list have a higher priority than products at the end.
  */

  while(fgets(line, (int) sizeof(line), fp) != NULL)
    if(strncmp(line, product, strlen(product)) == 0 && strstr(line, ":")
       != NULL)
      {
	if(strtok_r(line, ".", &lasts) != NULL)
	  variable = strtok_r(NULL, ":", &lasts);

	if((value = strtok_r(NULL, ":", &lasts)) == NULL)
	  value = (char *) "";

	do
	  {
	    if(value != NULL && strlen(value) > 0 &&
	       value[strlen(value) - 1] == '\n')
	      value[strlen(value) - 1] = '\0';

#ifdef DEBUG
	    (void) fprintf(stderr, "%s=%s\n", variable, value);
#endif

	    if(updatevariable(variable, value, flags, action) != 0)
	      {
		rc = 1;
		break;
	      }

	    value = strtok_r(NULL, ":", &lasts);
	  }
	while(rc == 0 && value != NULL);

	if(rc != 0)
	  break;
      }
    else
      break;

  return rc;
}

static int updatevariable(const char *variable, const char *value,
			  const struct flags_struct *flags, const int action)
{
  int rc = 0;
  char envact[MAX_LINE_LENGTH];

  if(variable == NULL || strlen(variable) == 0 ||
     value == NULL)
    ;
  else if(strcmp(variable, "PATH") == 0)
    rc = allocenv(&PATH, value, action, flags);
  else if(strcmp(variable, "MANPATH") == 0)
    {
      if(flags->no_manpath == 0)
	rc = allocenv(&MANPATH, value, action, flags);
    }
  else if(strcmp(variable, "LD_LIBRARY_PATH") == 0)
    {
      if(flags->no_ld_library_path == 0)
	rc = allocenv(&LD_LIBRARY_PATH, value, action, flags);
    }
  else if(strcmp(variable, "XFILESEARCHPATH") == 0)
    {
      if(flags->no_xfilesearchpath == 0)
	rc = allocenv(&XFILESEARCHPATH, value, action, flags);
    }
  else
    {
      /*
      ** Update the sourced file.
      */

      if(action == ADD_PATH)
	{
	  if(validatePath(value, flags) != 0)
	    (void) fprintf(_stdout_, "echo \"Warning: %s "
			   "is not a valid path.\"\n", value);

	  if(flags->shell_type == SH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == CSH)
	    (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			    variable, value);
	  else if(flags->shell_type == KSH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == BASH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == TCSH)
	    (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			    variable, value);
	}
      else
	{
	  if(flags->shell_type == SH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == CSH)
	    (void) snprintf(envact, sizeof(envact), "unsetenv %s", variable);
	  else if(flags->shell_type == KSH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == BASH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == TCSH)
	    (void) snprintf(envact, sizeof(envact), "unsetenv %s", variable);
	}

      if(flags->pretend)
	(void) fprintf(_stdout_, "echo \"%s\"\n", envact);
      else
	(void) fprintf(_stdout_, "%s\n", envact);
    }

  return rc;
}

static int allocenv(char **envvar, const char *value, const int action,
		    const struct flags_struct *flags)
{
  int rc = 0;
  int found = 0;
  char *tmp = NULL;
  char *token = NULL;
  size_t size = 0;

  if(*envvar == NULL)
    goto done_label;
  else if(action == ADD_PATH && strlen(value) > 0)
    size = strlen(*envvar) + strlen(value) + strlen(":") + 1;
  else
    size = strlen(*envvar) + 1;

  if((tmp = malloc(size)) == NULL)
    {
      rc = 1;
      goto done_label;
    }

  (void) memset(tmp, '\0', size);

  if(action == ADD_PATH && strlen(value) > 0)
    {
      if(validatePath(value, flags) != 0)
	(void) fprintf(_stdout_, "echo \"Warning: %s "
		       "is not a valid path.\"\n", value);

      (void) strcat(tmp, value);
      (void) strcat(tmp, ":");

#ifdef DEBUG
      (void) fprintf(stderr, "PATH1 = %s\n", value);
#endif
    }

  token = strtok(*envvar, ":");

  while(token != NULL)
    {
      if((found = strcmp(token, value)) != 0)
	(void) strcat(tmp, token);

      if((token = strtok(NULL, ":")) != NULL && found != 0)
	(void) strcat(tmp, ":");
    }

  if(tmp[strlen(tmp) - 1] == ':')
    tmp[strlen(tmp) - 1] = '\0';

  free(*envvar);
  size = strlen(tmp) + 1;

  if((*envvar = malloc(size)) == NULL)
    {
      rc = 1;
      goto done_label;
    }
  else
    (void) snprintf(*envvar, size, "%s", tmp);

 done_label:
  free(tmp);
  return rc;
}
