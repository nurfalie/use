#include "use.h"

static int detached_exists(const struct flags_struct *flags,
			   const char *value)
{
  int i = 0;

  if(!flags || !value)
    return -1;

  for(i = 0; i < flags->items_detached; i++)
    if(strcmp(flags->detached[i], value) == 0)
      return i;

  return -1;
}

static int used_exists(const struct flags_struct *flags,
		       const char *value)
{
  int i = 0;

  if(!flags || !value)
    return -1;

  for(i = 0; i < flags->items_used; i++)
    if(strcmp(flags->used[i], value) == 0)
      return i;

  return -1;
}

int validate(const int argc,
	     char *argv[],
	     struct flags_struct *flags)
{
  int i = 0;
  int rc = 0;

  if(!_stdout_ || !argv || !flags)
    {
      rc = 1;
      goto done_label;
    }

  for(i = 0; i < argc; i++)
    if(argv[i] && strcmp(argv[i], "-q") == 0)
      {
	flags->quiet = 1;
	break;
      }

  for(argv++; *argv != 0; argv++)
    {
      if(strcmp(*argv, "-a") == 0)
	{
	  if(argc > 4)
	    rc = 1;

	  flags->about = 1;
	}
      else if(strcmp(*argv, "-d") == 0)
	{
	  argv++;

	  if(*argv == 0)
	    {
	      rc = 1;
	      break;
	    }

	  if(used_exists(flags, *argv) != -1)
	    {
	      rc = 1;

	      if(!flags->quiet)
		(void) fprintf(_stdout_,
			       "echo \"Error: %s cannot be both detached "
			       "and used.\"\n", *argv);
	    }
	  else if(flags->items_detached < MAX_PRODUCTS)
	    {
	      if(detached_exists(flags, *argv) == -1)
		{
		  (void) memset
		    (flags->detached[flags->items_detached], 0,
		     sizeof(flags->detached[flags->items_detached]));
		  (void) snprintf(flags->detached[flags->items_detached],
				  sizeof(flags->detached[0]), "%s", *argv);
		  flags->items_detached += 1;
		}
	      else if(!flags->quiet)
		(void) fprintf(_stdout_, "echo \"Warning: %s is a "
			       "duplicate, ignoring.\"\n",
			       *argv);
	    }
	  else if(!flags->quiet)
	    (void) fprintf(_stdout_, "echo \"Warning: too many items "
			   "to detach, %s ignored.\"\n",
			   *argv);
	}
      else if(strcmp(*argv, "-l") == 0)
	flags->list = 1;
      else if(strcmp(*argv, "-n") == 0)
	{
	  argv++;

	  if(*argv == 0)
	    {
	      rc = 1;
	      break;
	    }

	  if(strcmp(*argv, "PATH") == 0)
	    flags->no_path = 1;
	  else if(strcmp(*argv, "MANPATH") == 0)
	    flags->no_manpath = 1;
#if defined(__APPLE__) || defined(__MACH__)
	  else if(strcmp(*argv, "DYLD_LIBRARY_PATH") == 0)
#else
	  else if(strcmp(*argv, "LD_LIBRARY_PATH") == 0)
#endif
	    flags->no_ld_library_path = 1;
	  else if(strcmp(*argv, "XFILESEARCHPATH") == 0)
	    flags->no_xfilesearchpath = 1;
	  else
	    {
	      rc = 1;

	      if(!flags->quiet)
		(void) fprintf(_stdout_,
			       "echo \"Error: %s is not a recognized "
			       "environment variable.\"\n",
			       *argv);
	    }
	}
      else if(strcmp(*argv, "-p") == 0)
	flags->pretend = 1;
      else if(strcmp(*argv, "-q") == 0)
	{
	  if(flags->list || flags->about)
	    rc = 1;
	}
      else if(strcmp(*argv, "-s") == 0)
	{
	  argv++;

	  if(*argv == 0)
	    {
	      rc = 1;
	      break;
	    }

	  if(strcmp(*argv, "SH") == 0)
	    flags->shell_type = SH;
	  else if(strcmp(*argv, "CSH") == 0)
	    flags->shell_type = CSH;
	  else if(strcmp(*argv, "KSH") == 0)
	    flags->shell_type = KSH;
	  else if(strcmp(*argv, "BASH") == 0)
	    flags->shell_type = BASH;
	  else if(strcmp(*argv, "TCSH") == 0)
	    flags->shell_type = TCSH;
	  else
	    {
	      rc = 1;

	      if(!flags->quiet)
		(void) fprintf(_stdout_, "echo \"Error: unknown shell "
			       "type %s.\"\n", *argv);
	    }
	}
      else if(strcmp(*argv, "-t") == 0)
	{
	  argv++;

	  if(*argv == 0)
	    {
	      rc = 1;
	      break;
	    }

	  if((_usefp_ = fopen(*argv, "r")) == 0)
	    {
	      rc = 1;

	      if(!flags->quiet)
		(void) fprintf(_stdout_,
			       "echo \"Error: %s cannot be accessed.", *argv);
	    }
	}
      else if(strcmp(*argv, "-u") == 0)
	{
	  argv++;

	  if(*argv == 0)
	    {
	      rc = 1;
	      break;
	    }

	  if(detached_exists(flags, *argv) != -1)
	    {
	      rc = 1;

	      if(!flags->quiet)
		(void) fprintf(_stdout_,
			       "echo \"Error: %s cannot be both "
			       "detached and used.\"\n", *argv);
	    }
	  else if(flags->items_used < MAX_PRODUCTS)
	    {
	      if(used_exists(flags, *argv) == -1)
		{
		  (void) memset
		    (flags->used[flags->items_used], 0,
		     sizeof(flags->used[flags->items_used]));
		  (void) snprintf(flags->used[flags->items_used],
				  sizeof(flags->used[0]), "%s", *argv);
		  flags->items_used += 1;
		}
	      else if(!flags->quiet)
		(void) fprintf(_stdout_, "echo \"Warning: %s is a "
			       "duplicate, ignoring.\"\n",
			       *argv);
	    }
	  else if(!flags->quiet)
	    (void) fprintf(_stdout_,
			   "echo \"Warning: too many items to use, %s "
			   "ignored.\"\n", *argv);
	}
      else if(!flags->quiet)
	(void) fprintf(_stdout_, "echo \"Warning: unknown option %s, "
		       "ignoring.\"\n", *argv);
    }

  if(flags->shell_type == 0)
    rc = 1;
  else if(flags->list > 0 && flags->about > 0) /* Both -a and -l were used. */
    rc = 1;
  else if(flags->list == 1 || flags->about == 1)
    {
    }
  else if(flags->items_used == 0 && flags->items_detached == 0)
    rc = 1;

  if(rc != 0)
    if(_usefp_)
      {
	fclose(_usefp_);
	_usefp_ = 0;
      }

 done_label:
  return rc;
}

int validatePath(const char *path,
		 const struct flags_struct *flags)
{
  int rc = 0;
  struct stat sb;

  if(!flags || !path)
    rc = 1;
  else
    {
      if(!flags->quiet)
	if(stat(path, &sb) != 0)
	  rc = 1;
    }

  return rc;
}
