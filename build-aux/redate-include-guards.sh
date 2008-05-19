#! /bin/sh --

set -e

die()
{
  echo >&2 "$1"
  exit 1
}

test "${#}" = "1" || die "Usage: ${0} ioxx_top_srcdir"
cd "${1}"
test -f "include/ioxx.hpp" || die "this is not the ioxx top_srcdir"

today=`date --iso-8601 | tr - _`

echo -n "redate include guards"
for n in `find . -name *.hpp`; do
  echo -n " ${n##./}"
  sed -i -e "s/200[78]_[0-9][0-9]_[0-9][0-9]/${today}/g" "${n}"
done
echo " done"


