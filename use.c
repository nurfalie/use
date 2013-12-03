/*
** Copyright (c) 2006, 2007, 2013 Alexis Megas
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

static char *PATH = 0;
static char *MANPATH = 0;
static char *LD_LIBRARY_PATH = 0;
static char *XFILESEARCHPATH = 0;

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
  char *tmp = 0;
  char line[MAX_LINE_LENGTH];
  char filename[PATH_MAX];
  FILE *fp = 0;
  struct flags_struct flags;

  (void) memset(filename, 0, sizeof(filename));
  (void) snprintf(filename, sizeof(filename), "%s/.use.sourceme.%ud.%ud",
		  TEMPDIR, getuid(), getpid());

  if((_stdout_ = fopen(filename, "a+")) == 0)
    {
      (void) fprintf(stdout, "/dev/null");
      return EXIT_FAILURE;
    }
  else
    (void) fprintf(stdout, "%s", filename);

  (void) setvbuf(_stdout_, (char *) 0, _IONBF, 0);

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
      if((fp = fopen(USETABLE, "r")) == 0)
	{
	  rc = 1;
	  (void) fprintf(_stdout_, "echo \"Error: unable to open %s.\"",
			 USETABLE);
	}
      else
	{
	  while(fgets(line, (int) sizeof(line), fp) != 0)
	    if(strstr(line, ".description") != 0 &&
	       strstr(line, ":") != 0 && line[0] != '#')
	      {
		tmp = strstr(line, ":") + 1;

		if(tmp[strlen(tmp) - 1] == '\n')
		  tmp[strlen(tmp) - 1] = 0;

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
      (void) memset(flags->used[i], 0, sizeof(flags->used[0]));
      (void) memset(flags->detached[i], 0, sizeof(flags->detached[0]));
    }
}

static int use(struct flags_struct *flags)
{
  int i = 0;
  int rc = 0;
  int found = 0;
  int dousevalues[4]; /* This should be the same size as stdvariables. */
  char *tmp = 0;
  char line[MAX_LINE_LENGTH];
  char envact[MAX_LINE_LENGTH];
  char *product = 0;
  char *stdvalues[4]; /* This should be the same size as stdvariables. */
  char *stdvariables[] = {"PATH",
			  "MANPATH",
			  "LD_LIBRARY_PATH",
			  "XFILESEARCHPATH"};
  size_t size = 0;
  FILE *fp = 0;

  if(flags->no_path == 0)
    {
      if((tmp = getenv("PATH")) != 0)
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(PATH);
      PATH = 0;

      if(size > 0)
	{
	  PATH = (char *) malloc(size);
	  (void) memset(PATH, 0, size);
	}

      if(PATH != 0)
	{
	  if(tmp != 0)
	    (void) strncpy(PATH, tmp, size - 1);
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
      if((tmp = getenv("MANPATH")) != 0)
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(MANPATH);
      MANPATH = 0;

      if(size > 0)
	{
	  MANPATH = (char *) malloc(size);
	  (void) memset(MANPATH, 0, size);
	}

      if(MANPATH != 0)
	{
	  if(tmp != 0)
	    (void) strncpy(MANPATH, tmp, size - 1);
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
      if((tmp = getenv("LD_LIBRARY_PATH")) != 0)
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(LD_LIBRARY_PATH);
      LD_LIBRARY_PATH = 0;

      if(size > 0)
	{
	  LD_LIBRARY_PATH = (char *) malloc(size);
	  (void) memset(LD_LIBRARY_PATH, 0, size);
	}

      if(LD_LIBRARY_PATH != 0)
	{
	  if(tmp != 0)
	    (void) strncpy(LD_LIBRARY_PATH, tmp, size - 1);
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
      if((tmp = getenv("XFILESEARCHPATH")) != 0)
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(XFILESEARCHPATH);
      XFILESEARCHPATH = 0;

      if(size > 0)
	{
	  XFILESEARCHPATH = (char *) malloc(size);
	  (void) memset(XFILESEARCHPATH, 0, size);
	}

      if(XFILESEARCHPATH != 0)
	{
	  if(tmp != 0)
	    (void) strncpy(XFILESEARCHPATH, tmp, size - 1);
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

  if((fp = fopen(USETABLE, "r")) != 0)
    {
      for(i = flags->items_detached - 1; i >= 0; i--)
	{
	  found = 0;

	  while(fgets(line, (int) sizeof(line), fp) != 0)
	    if(strstr(line, ".description") != 0 &&
	       strstr(line, ":") != 0 && line[0] != '#')
	      {
		product = line;

		if(strstr(product, ".") != 0)
		  {
		    product[strlen(product) -
			    strlen(strstr(product, "."))] = 0;

		    if(strcmp(flags->detached[i], product) == 0)
		      {
			found = 1;

			if(prepare(fp, product, flags, DELETE_PATH) != 0)
			  {
			    rc = 1;

			    if(!flags->quiet)
			      (void) fprintf
				(_stdout_,
				 "echo \"Error: unable to prepare "
				 "containers for product %s.\"\n",
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

	  while(fgets(line, (int) sizeof(line), fp) != 0)
	    if(strstr(line, ".description") != 0 &&
	       strstr(line, ":") != 0 && line[0] != '#')
	      {
		product = line;

		if(strstr(product, ".") != 0)
		  {
		    product[strlen(product) -
			    strlen(strstr(product, "."))] = 0;

		    if(strcmp(flags->used[i], product) == 0)
		      {
			found = 1;

			if(prepare(fp, product, flags, ADD_PATH) != 0)
			  {
			    rc = 1;

			    if(!flags->quiet)
			      (void) fprintf
				(_stdout_,
				 "echo \"Error: unable to prepare "
				 "containers for product %s.\"\n",
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

  if(fp != 0)
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
	if(stdvalues[i] != 0 &&
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
  char *value = 0;
  char *variable = 0;

  /*
  ** The order of the products listed is significant. Products at the
  ** front of the list have a higher priority than products at the end.
  */

  if(flags == 0 || fp == 0 || product == 0 || strlen(product) == 0)
    {
      rc = 1;
      return rc;
    }

  while(fgets(line, (int) sizeof(line), fp) != 0)
    if(strncmp(line, product, strlen(product)) == 0 &&
       strstr(line, ":") != 0)
      {
	if(strtok_r(line, ".", &lasts) != 0)
	  variable = strtok_r(0, ":", &lasts);

	if((value = strtok_r(0, ":", &lasts)) == 0)
	  value = (char *) "";

	do
	  {
	    if(value != 0 && strlen(value) > 0 &&
	       value[strlen(value) - 1] == '\n')
	      value[strlen(value) - 1] = 0;

#ifdef DEBUG
	    if(value && variable)
	      (void) fprintf(stderr, "%s=%s\n", variable, value);
#endif

	    if(updatevariable(variable, value, flags, action) != 0)
	      {
		rc = 1;
		break;
	      }

	    value = strtok_r(0, ":", &lasts);
	  }
	while(rc == 0 && value != 0);

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

  if(flags == 0 || value == 0 || variable == 0 || strlen(variable) == 0)
    {
    }
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
  char *tmp = 0;
  char *token = 0;
  size_t size = 0;

  if(*envvar == 0)
    goto done_label;
  else if(action == ADD_PATH && value && strlen(value) > 0)
    size = strlen(*envvar) + strlen(value) + strlen(":") + 1;
  else
    size = strlen(*envvar) + 1;

  if((tmp = malloc(size)) == 0)
    {
      rc = 1;
      goto done_label;
    }

  (void) memset(tmp, 0, size);

  if(action == ADD_PATH && strlen(value) > 0)
    {
      if(validatePath(value, flags) != 0)
	(void) fprintf(_stdout_, "echo \"Warning: %s "
		       "is not a valid path.\"\n", value);

      (void) strncat(tmp, value, size - strlen(tmp) - 1);
      (void) strncat(tmp, ":", size - strlen(tmp) - 1);

#ifdef DEBUG
      (void) fprintf(stderr, "PATH1 = %s\n", value);
#endif
    }

  token = strtok(*envvar, ":");

  while(token != 0)
    {
      if((found = strcmp(token, value)) != 0)
	(void) strncat(tmp, token, size - strlen(tmp) - 1);

      if((token = strtok(0, ":")) != 0 && found != 0)
	(void) strncat(tmp, ":", size - strlen(tmp) - 1);
    }

  if(tmp[strlen(tmp) - 1] == ':')
    tmp[strlen(tmp) - 1] = 0;

  free(*envvar);
  size = strlen(tmp) + 1;

  if((*envvar = malloc(size)) == 0)
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
