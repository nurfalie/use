#!/bin/csh -x

set BACKUP_DIR = /love/scsi/backup.d/use.d

if ( ! -d $BACKUP_DIR ) then
    mkdir $BACKUP_DIR
endif

cp -p *.c $BACKUP_DIR/.
cp -p *.h $BACKUP_DIR/.
cp -p Makefile $BACKUP_DIR/.
cp -p backup.csh $BACKUP_DIR/.
cp -p use.* $BACKUP_DIR/.
cp -p INSTALL $BACKUP_DIR/.
cp -p *.pl $BACKUP_DIR/.

rm -f $BACKUP_DIR/use.bin
rm -f $BACKUP_DIR/use_tmp.h
rm -f $BACKUP_DIR/Makefile.tmp
