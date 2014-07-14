#!/usr/bin/perl -w
# Alexis Megas (c) 2006.

use File::Path;
use File::stat;
use POSIX;

# Macros.

$EXIT_SUCCESS = 0;
$EXIT_FAILURE = 1;

# Determine the location of the use.table file.

$val = $usetabledir = "/usr/local/share";

do
{
    printf("Where would you like the use.table file to reside? [%s] ",
	   $usetabledir);
    chop($val = <STDIN>);

    if(!$val)
    {
	$val = $usetabledir;
    }

    if(! -d $val)
    {
	printf("\"%s\" doesn't appear to exist or is an invalid " .
	       "directory.\n", $val);
    }
}
while(! -d $val);

$usetabledir = $val . "/use.table";

# Create use_temp.h in the current directory.

$use_tmp = "#ifndef _use_temp_h_\n" .
    "#define _use_temp_h_\n" .
    "#define USETABLE \"" . $usetabledir . "\"\n" .
    "#endif\n";

if(!open(OUTPUT, "+> ./use_tmp.h"))
{
    printf("Unable to open() %s. Exiting.\n", "./use_tmp.h");
    exit($EXIT_FAILURE);
}

print OUTPUT $use_tmp;
close(OUTPUT);

# Determine the location of the executables.

$val = $bindir = "/usr/local/bin";

do
{
    printf("Where would you like the executable files to reside? [%s] ",
	   $bindir);
    chop($val = <STDIN>);

    if(!$val)
    {
	$val = $bindir;
    }

    if(! -d $val)
    {
	printf("\"%s\" doesn't appear to exist or is an invalid " .
	       "directory.\n", $val);
    }
}
while(! -d $val);

$bindir = $val;

# Determine the location of the manual page.

$val = $mandir = "/usr/share/man/man1";

do
{
    printf("Where would you like the manual page to reside? [%s] ", $mandir);
    chop($val = <STDIN>);

    if(!$val)
    {
	$val = $mandir;
    }

    if(! -d $val)
    {
	printf("\"%s\" doesn't appear to exist or is an invalid " .
	       "directory.\n", $val);
    }
}
while(! -d $val);

$mandir = $val;

# Create Makefile.tmp.

$mktmp = "INSTALL_MANPATH = " . $mandir . "\n" .
    "INSTALL_PATH = " . $bindir . "\n" .
    "INSTALL_TABLEFULLPATH = " . $usetabledir . "\n";

if(!open(OUTPUT, "+> ./Makefile.tmp"))
{
    printf("Unable to open() %s. Exiting.\n", "./Makefile.tmp");
    exit($EXIT_FAILURE);
}

print OUTPUT $mktmp;
printf("Please do not forget to set the correct privileges after " .
       "installing the files!\n");
close(OUTPUT);
