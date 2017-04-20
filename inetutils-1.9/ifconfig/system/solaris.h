/*
  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
  2010, 2011 Free Software Foundation, Inc.

  This file is part of GNU Inetutils.

  GNU Inetutils is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  GNU Inetutils is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see `http://www.gnu.org/licenses/'. */

/* Written by Marcus Brinkmann.  */

#ifndef IFCONFIG_SYSTEM_SOLARIS_H
# define IFCONFIG_SYSTEM_SOLARIS_H

# include "../printif.h"
# include "../options.h"
# include <sys/sockio.h>


/* XXX: Gross. Have autoconf check and put in system.h or so.
   The correctness is documented in Solaris 2.7, if_tcp(7p).  */

# define ifr_mtu ifr_metric


/* Option support.  */

struct system_ifconfig
{
  int valid;
};


/* Output format support.  */

# define SYSTEM_FORMAT_HANDLER \
  {"solaris", fh_nothing},

#endif
