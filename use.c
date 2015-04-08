/*
** Copyright (c) 2006 - present, Alexis Megas.
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
#include "validate.h"

static char *LD_LIBRARY_PATH = 0;
static char *MANPATH = 0;
static char *PATH = 0;
static char *XFILESEARCHPATH = 0;

static int allocenv(char **envvar,
		    const char *value,
		    const int action,
		    const struct flags_struct *flags);
static int prepare(FILE *fp,
		   const char *product,
		   const struct flags_struct *flags,
		   const int action);
static void reset(struct flags_struct *flags);
static int updatevariable(const char *variable,
			  const char *value,
			  const struct flags_struct *flags,
			  const int action);
static int use(struct flags_struct *flags);

int main(int argc, char *argv[])
{
  FILE *fp = 0;
  char filename[PATH_MAX];
  char line[MAX_LINE_LENGTH];
  char *tmp = 0;
  int fd = -1;
  int rc = 0;
  struct flags_struct flags;
  struct passwd *pw = getpwuid(getuid());

  (void) memset(TEMPDIR, 0, sizeof(TEMPDIR));

  if(pw && pw->pw_dir)
    (void) snprintf(TEMPDIR, sizeof(TEMPDIR), "%s", pw->pw_dir);
  else
    (void) snprintf(TEMPDIR, sizeof(TEMPDIR), "%s", "/tmp");

  (void) memset(filename, 0, sizeof(filename));
  (void) snprintf
    (filename, sizeof(filename), "%s/.use.sourceme.%ld.%ldXXXXXX",
     TEMPDIR, (long) getuid(), (long) getpid());

  if((fd = mkstemp(filename)) == -1)
    {
      if(stdout)
	fprintf(stdout, "%s", "");

      return EXIT_FAILURE;
    }

  if(!(_stdout_ = fdopen(fd, "a+")))
    {
      if(stdout)
	(void) fprintf(stdout, "%s", "");

      return EXIT_FAILURE;
    }
  else if(stdout)
    (void) fprintf(stdout, "%s", filename);

  (void) setvbuf(_stdout_, 0, _IONBF, (size_t) 0);
  _usefp_ = 0;

  /*
  ** Reset the flags structure.
  */

  reset(&flags);

  if(validate(argc, argv, &flags) != 0)
    {
      /*
      ** Print the usage information.
      */

      rc = 1;

      if(!flags.quiet)
	(void) fprintf(_stdout_, "%s", "echo \"use: incorrect usage.\"\n");

      goto done_label;
    }

  if(flags.list)
    {
      if(_usefp_ == 0 && !(fp = fopen(USETABLE, "r")))
	{
	  rc = 1;
	  (void) fprintf(_stdout_, "echo \"Error: unable to open %s.\"",
			 USETABLE);
	}
      else
	{
	  if(_usefp_)
	    fp = _usefp_;

	  (void) memset(line, 0, sizeof(line));

	  while(fgets(line, (int) sizeof(line), fp))
	    {
	      if(strstr(line, ".description") &&
		 strstr(line, ":") && line[0] != '#')
		{
		  tmp = strstr(line, ":") + 1;

		  if(tmp && strlen(tmp) > 0 && tmp[strlen(tmp) - 1] == '\n')
		    tmp[strlen(tmp) - 1] = 0;

		  if(tmp)
		    (void) fprintf(_stdout_, "echo \"%s\"\n", tmp);
		}

	      (void) memset(line, 0, sizeof(line));
	    }

	  (void) fclose(fp);
	  fp = 0;
	}

      goto done_label;
    }
  else if(flags.about)
    {
      (void) fprintf(_stdout_, "%s", "echo \"use\"\n");
      (void) fprintf(_stdout_, "echo \"Version: %s\"\n", VERSION);
      (void) fprintf(_stdout_, "echo \"Compilation Date & Time: %s %s\"\n",
		     __DATE__, __TIME__);
      (void) fprintf(_stdout_, "echo \"Temporary Directory: %s\"\n", TEMPDIR);
      (void) fprintf(_stdout_, "echo \"Use Table: %s\"\n", USETABLE);
      goto done_label;
    }

  rc = use(&flags);
  free(LD_LIBRARY_PATH);
  free(MANPATH);
  free(PATH);
  free(XFILESEARCHPATH);

 done_label:
  if(_stdout_)
    {
      (void) fflush(_stdout_);

      /*
      ** Also closes fd.
      */

      (void) fclose(_stdout_);
    }

  if(rc != 0)
    return EXIT_FAILURE;
  else
    return EXIT_SUCCESS;
}

static void reset(struct flags_struct *flags)
{
  int i = 0;

  if(flags)
    {
      flags->about = 0;
      flags->items_detached = 0;
      flags->items_used = 0;
      flags->list = 0;
      flags->no_ld_library_path = 0;
      flags->no_manpath = 0;
      flags->no_path = 0;
      flags->no_xfilesearchpath = 0;
      flags->pretend = 0;
      flags->quiet = 0;
      flags->shell_type = 0;

      for(i = 0; i < MAX_PRODUCTS; i++)
	{
	  (void) memset(flags->detached[i], 0, sizeof(flags->detached[0]));
	  (void) memset(flags->used[i], 0, sizeof(flags->used[0]));
	}
    }
}

static int use(struct flags_struct *flags)
{
  FILE *fp = 0;
  char envact[MAX_LINE_LENGTH];
  char line[MAX_LINE_LENGTH];
  char *product = 0;
  char *stdvalues[4]; /* This must be the same size as stdvariables. */
  char *stdvariables[] = 
    {
#if defined(__APPLE__) || defined(__MACH__)
      "DYLD_LIBRARY_PATH",
#else
      "LD_LIBRARY_PATH",
#endif
      "MANPATH",
      "PATH",
      "XFILESEARCHPATH"
    };
  char *tmp = 0;
  int dousevalues[4]; /* This must be the same size as stdvariables. */
  int i = 0;
  int rc = 0;
  size_t size = 0;

  if(!_stdout_ || !flags)
    {
      rc = 1;
      goto done_label;
    }

  if(flags->no_ld_library_path == 0)
    {
#if defined(__APPLE__) || defined(__MACH__)
      if((tmp = getenv("DYLD_LIBRARY_PATH")))
#else
      if((tmp = getenv("LD_LIBRARY_PATH")))
#endif
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(LD_LIBRARY_PATH);
      LD_LIBRARY_PATH = 0;

      if(size > 0)
	{
	  LD_LIBRARY_PATH = malloc(size);
	  (void) memset(LD_LIBRARY_PATH, 0, size);
	}

      if(LD_LIBRARY_PATH && tmp)
	(void) strncpy(LD_LIBRARY_PATH, tmp, size - 1);
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
#if defined(__APPLE__) || defined(__MACH__)
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to allocate memory for "
			   "DYLD_LIBRARY_PATH or tmp is empty.\"\n");
#else
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to allocate memory for "
			   "LD_LIBRARY_PATH or tmp is empty.\"\n");
#endif

	  goto done_label;
	}
    }
  else if(!flags->quiet)
#if defined(__APPLE__) || defined(__MACH__)
    (void) fprintf(_stdout_, "%s",
		   "echo \"Warning: DYLD_LIBRARY_PATH not "
		   "updated.\"\n");
#else
    (void) fprintf(_stdout_, "%s",
		   "echo \"Warning: LD_LIBRARY_PATH not "
		   "updated.\"\n");
#endif

  if(flags->no_manpath == 0)
    {
      if((tmp = getenv("MANPATH")))
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(MANPATH);
      MANPATH = 0;

      if(size > 0)
	{
	  MANPATH = malloc(size);
	  (void) memset(MANPATH, 0, size);
	}

      if(MANPATH && tmp)
	(void) strncpy(MANPATH, tmp, size - 1);
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to allocate "
			   "memory for MANPATH or tmp is empty.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "%s",
		   "echo \"Warning: MANPATH not updated.\"\n");

  if(flags->no_path == 0)
    {
      if((tmp = getenv("PATH")))
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(PATH);
      PATH = 0;

      if(size > 0)
	{
	  PATH = malloc(size);
	  (void) memset(PATH, 0, size);
	}

      if(PATH && tmp)
	(void) strncpy(PATH, tmp, size - 1);
      else
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to allocate "
			   "memory for PATH or tmp is empty.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "%s", "echo \"Warning: PATH not updated.\"\n");

  if(flags->no_xfilesearchpath == 0)
    {
      if((tmp = getenv("XFILESEARCHPATH")))
	size = strlen(tmp) + 1;
      else
	size = 0;

      free(XFILESEARCHPATH);
      XFILESEARCHPATH = 0;

      if(size > 0)
	{
	  XFILESEARCHPATH = malloc(size);
	  (void) memset(XFILESEARCHPATH, 0, size);
	}

      if(XFILESEARCHPATH && tmp)
	(void) strncpy(XFILESEARCHPATH, tmp, size - 1);
      else if(size > 0)
	{
	  rc = 1;

	  if(!flags->quiet)
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to allocate memory for "
			   "XFILESEARCHPATH or tmp is empty.\"\n");

	  goto done_label;
	}
    }
  else if(!flags->quiet)
    (void) fprintf(_stdout_, "%s",
		   "echo \"Warning: XFILESEARCHPATH not "
		   "updated.\"\n");

  if(_usefp_)
    fp = _usefp_;
  else
    fp = fopen(USETABLE, "r");

  if(fp)
    {
      int found = 0;

      for(i = flags->items_detached - 1; i >= 0; i--)
	{
	  found = 0;
	  (void) memset(line, 0, sizeof(line));

	  while(fgets(line, (int) sizeof(line), fp))
	    {
	      if(strstr(line, ".description") &&
		 strstr(line, ":") && line[0] != '#')
		{
		  product = line;

		  if(strstr(product, ".") &&
		     strlen(product) - strlen(strstr(product, ".")) > 0)
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
				   "containers for product %s. Please "
				   "review the use file.\"\n",
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
			{
			  if(_usefp_)
			    (void) fprintf(_stdout_,
					   "%s",
					   "echo \"Error: possible "
					   "misconfiguration with "
					   "the provided table file.\"\n");
			  else
			    (void) fprintf(_stdout_,
					   "echo \"Error: possible "
					   "misconfiguration with %s.\"\n",
					   USETABLE);
			}

		      break;
		    }
		}

	      (void) memset(line, 0, sizeof(line));
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
	  (void) memset(line, 0, sizeof(line));

	  while(fgets(line, (int) sizeof(line), fp))
	    {
	      if(strstr(line, ".description") &&
		 strstr(line, ":") && line[0] != '#')
		{
		  product = line;

		  if(strstr(product, ".") &&
		     strlen(product) - strlen(strstr(product, ".")) > 0)
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
				   "containers for product %s. Please "
				   "review the use file.\"\n",
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
			{
			  if(_usefp_)
			    (void) fprintf(_stdout_,
					   "%s",
					   "echo \"Error: possible "
					   "misconfiguration with "
					   "the provided table file.\"\n");
			  else
			    (void) fprintf(_stdout_,
					   "echo \"Error: possible "
					   "misconfiguration with %s.\"\n",
					   USETABLE);
			}

		      break;
		    }
		}

	      (void) memset(line, 0, sizeof(line));
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
	{
	  if(_usefp_)
	    (void) fprintf(_stdout_, "%s",
			   "echo \"Error: unable to "
			   "open the provided table file.\"\n");
	  else
	    (void) fprintf(_stdout_, "echo \"Error: unable to "
			   "open %s.\"\n", USETABLE);
	}
    }

 done_label:

  if(fp)
    {
      (void) fclose(fp);
      fp = 0;
    }

  if(rc == 0)
    {
      /*
      ** Update the sourced file.
      */

      stdvalues[0] = LD_LIBRARY_PATH;
      stdvalues[1] = MANPATH;
      stdvalues[2] = PATH;
      stdvalues[3] = XFILESEARCHPATH;
      dousevalues[0] = flags->no_ld_library_path;
      dousevalues[1] = flags->no_manpath;
      dousevalues[2] = flags->no_path;
      dousevalues[3] = flags->no_xfilesearchpath;

      for(i = 0; i < 4; i++)
	if(stdvalues[i] != 0 &&
	   strlen(stdvalues[i]) > 0) /* In case the -n option was used. */
	  {
	    (void) memset(envact, 0, sizeof(envact));

	    if(flags->shell_type == BASH)
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
	    else if(flags->shell_type == SH)
	      (void) snprintf(envact, sizeof(envact), "export %s=%s",
			      stdvariables[i],
			      stdvalues[i]);
	    else if(flags->shell_type == TCSH)
	      (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			      stdvariables[i],
			      stdvalues[i]);
	    else
	      continue;

	    if(flags->pretend)
	      (void) fprintf(_stdout_, "echo \"%s\"\n", envact);
	    else
	      (void) fprintf(_stdout_, "%s\n", envact);
	  }
	else if(dousevalues[i] == 0)
	  {
	    (void) memset(envact, 0, sizeof(envact));

	    if(flags->shell_type == BASH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == CSH)
	      (void) snprintf(envact, sizeof(envact), "unsetenv %s",
			      stdvariables[i]);
	    else if(flags->shell_type == KSH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == SH)
	      (void) snprintf(envact, sizeof(envact), "unset %s",
			      stdvariables[i]);
	    else if(flags->shell_type == TCSH)
	      (void) snprintf(envact, sizeof(envact), "unsetenv %s",
			      stdvariables[i]);
	    else
	      continue;

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
  char *lasts;
  char line[MAX_LINE_LENGTH];
  char *value = 0;
  char *variable = 0;
  int rc = 0;

  /*
  ** The order of the products listed is significant. Products at the
  ** front of the list have a higher priority than products at the end.
  */

  if(!flags || !fp || !product || strlen(product) == 0)
    {
      rc = 1;
      return rc;
    }

  (void) memset(line, 0, sizeof(line));

  while(fgets(line, (int) sizeof(line), fp))
    {
      if(strncmp(line, product, strlen(product)) == 0 && strstr(line, ":"))
	{
	  if(strtok_r(line, ".", &lasts))
	    variable = strtok_r(0, ":", &lasts);

	  if(!(value = strtok_r(0, ":", &lasts)))
	    value = "";

	  do
	    {
	      if(value && strlen(value) > 0 &&
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
	  while(rc == 0 && value);

	  if(rc != 0)
	    break;
	}
      else
	break;

      (void) memset(line, 0, sizeof(line));
    }

  return rc;
}

static int updatevariable(const char *variable, const char *value,
			  const struct flags_struct *flags, const int action)
{
  char envact[MAX_LINE_LENGTH];
  int rc = 0;

  if(!_stdout_ || !flags || !value || !variable || strlen(variable) == 0)
    {
      if(!_stdout_ || !flags)
	rc = 1;
    }
#if defined(__APPLE__) || defined(__MACH__)
  else if(strcmp(variable, "DYLD_LIBRARY_PATH") == 0)
#else
  else if(strcmp(variable, "LD_LIBRARY_PATH") == 0)
#endif
    {
      if(flags->no_ld_library_path == 0)
	rc = allocenv(&LD_LIBRARY_PATH, value, action, flags);
    }
  else if(strcmp(variable, "MANPATH") == 0)
    {
      if(flags->no_manpath == 0)
	rc = allocenv(&MANPATH, value, action, flags);
    }
  else if(strcmp(variable, "PATH") == 0)
    rc = allocenv(&PATH, value, action, flags);
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

      (void) memset(envact, 0, sizeof(envact));

      if(action == ADD_PATH)
	{
	  if(validatePath(value, flags) != 0)
	    (void) fprintf(_stdout_, "echo \"Warning: %s "
			   "is not a valid path.\"\n", value);

	  if(flags->shell_type == BASH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == CSH)
	    (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			    variable, value);
	  else if(flags->shell_type == KSH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == SH)
	    (void) snprintf(envact, sizeof(envact), "export %s=%s",
			    variable, value);
	  else if(flags->shell_type == TCSH)
	    (void) snprintf(envact, sizeof(envact), "setenv %s %s",
			    variable, value);
	}
      else
	{
	  if(flags->shell_type == BASH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == CSH)
	    (void) snprintf(envact, sizeof(envact), "unsetenv %s", variable);
	  else if(flags->shell_type == KSH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == SH)
	    (void) snprintf(envact, sizeof(envact), "unset %s", variable);
	  else if(flags->shell_type == TCSH)
	    (void) snprintf(envact, sizeof(envact), "unsetenv %s", variable);
	}

      if(strlen(envact) > 0)
	{
	  if(flags->pretend)
	    (void) fprintf(_stdout_, "echo \"%s\"\n", envact);
	  else
	    (void) fprintf(_stdout_, "%s\n", envact);
	}
    }

  return rc;
}

static int allocenv(char **envvar, const char *value, const int action,
		    const struct flags_struct *flags)
{
  char *tmp = 0;
  char *token = 0;
  int found = 0;
  int rc = 0;
  size_t size = 0;

  if(!_stdout_ || !flags)
    goto done_label;
  else if(action == ADD_PATH)
    {
      if(*envvar && value && strlen(value) > 0)
	size = strlen(*envvar) + strlen(value) + strlen(":") + 1;
      else if(value && strlen(value) > 0)
	size = strlen(value) + strlen(":") + 1;
    }
  else
    {
      if(*envvar)
	size = strlen(*envvar) + 1;
      else
	/*
	** Not an error.
	*/

	goto done_label;
    }

  if(size == 0)
    {
      rc = 1;
      goto done_label;
    }

  if(!(tmp = malloc(size)))
    {
      rc = 1;
      goto done_label;
    }

  (void) memset(tmp, 0, size);

  if(action == ADD_PATH && value && strlen(value) > 0)
    {
      if(validatePath(value, flags) != 0)
	(void) fprintf(_stdout_, "echo \"Warning: %s "
		       "is not a valid path.\"\n", value);

      if(size > strlen(tmp))
	(void) strncat(tmp, value, size - strlen(tmp) - 1);

      if(size > strlen(tmp))
	(void) strncat(tmp, ":", size - strlen(tmp) - 1);

#ifdef DEBUG
      (void) fprintf(stderr, "PATH1 = %s\n", value);
#endif
    }

  token = strtok(*envvar, ":");

  while(token != 0)
    {
      if(value && (found = strcmp(token, value)) != 0)
	if(size > strlen(tmp))
	  (void) strncat(tmp, token, size - strlen(tmp) - 1);

      if((token = strtok(0, ":")) && found != 0)
	if(size > strlen(tmp))
	  (void) strncat(tmp, ":", size - strlen(tmp) - 1);
    }

  if(strlen(tmp) > 0 && tmp[strlen(tmp) - 1] == ':')
    tmp[strlen(tmp) - 1] = 0;

  free(*envvar);
  size = strlen(tmp) + 1;

  if(!(*envvar = malloc(size)))
    {
      rc = 1;
      goto done_label;
    }
  else
    {
      (void) memset(*envvar, 0, size);
      (void) snprintf(*envvar, size, "%s", tmp);
    }

 done_label:
  free(tmp);
  return rc;
}
