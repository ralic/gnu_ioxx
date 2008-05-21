#! /bin/sh

exec tar ${1+"$@"} --owner=0 --group=0 --mtime="`date --iso` 00:00"
