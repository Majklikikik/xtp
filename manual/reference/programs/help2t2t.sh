#! /bin/bash

die() {
  echo -e "$*" >&2
  exit 1
}

[[ -z $1 || -z $2 ]] && die "${0##*/}: Missing argument"
prog="${1}"
[[ -f $2 ]] || die "${0##*/}: Could not open '$2'"
helpmsg="$(<$2)"

echo "$prog"
echo ${0##*/}
date
echo
echo '%!includeconf: config.t2t'
echo 
echo -e "++$prog++"
echo -e "label(prog:$prog)"

#Here comes the magic:
#-header in trash
#-remove prog/tools/gromacs version line
#-space at begining and end are removed
#-optionlines (-[^ ]) -> - ``--option`` text
#-usageline -> usage: ``code``
#-extra empty line before new section to close itemize
#-examplelines (^*) -> - ``line``
#-add cg.interaction in links
echo -e "$helpmsg" | sed \
   -e '1,/^please submit/d' \
   -e "/^\($prog\|votca_tools\|gromacs\|votca_csg\)/d" \
   -e 's/^[[:space:]]*//' \
   -e 's/[[:space:]]*$//' \
   -e '/^-[^ ]/s/\( \|)\) /\1`` /' \
   -e 's/ ``/``/' \
   -e '/^-[^ ].*``/s/^/- ``/' \
   -e '/^\* /s/\( \{2\}\|$\)/`` /' \
   -e '/^\*.*``/s/^\*[[:space:]]*/- ``/' \
   -e '/^\(Examples\|Usage\):/d' \
   -e "/^\(Allowed\|Calculators\|Specific\)/d"

