#       Copyright (c) 1987,1992 AT&T
#       All Rights Reserved

#       THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#       The copyright notice above does not evidence any
#       actual or intended publication of such source code.

# @(#)startup.sh        1.1.1.4 (7/17/92)

exec ${DMDTOOLS:-${TC630:-$TOOLS}}/lib/dmdtools/dmdtool `basename $0` ${1+"$@"}
