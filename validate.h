#ifndef _USE_VALIDATE_H_
#define _USE_VALIDATE_H_

int validate(const int argc,
	     char *argv[],
	     struct flags_struct *flags);
int validatePath(const char *path,
		 const struct flags_struct *flags);

#endif
