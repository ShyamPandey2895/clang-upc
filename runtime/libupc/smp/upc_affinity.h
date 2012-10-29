/* Copyright (c) 2008, 2009, 2010, 2011
   Free Software Foundation, Inc. 
   This file is part of the UPC runtime library.
   Written by Gary Funck <gary@intrepid.com>
   and Nenad Vukicevic <nenad@intrepid.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */


#ifndef _UPC_AFFINITY_H_
#define _UPC_AFFINITY_H_

extern void __upc_affinity_cpu_avoid_free (const upc_cpu_avoid_p);
extern upc_cpu_avoid_p __upc_affinity_cpu_avoid_new (void);
extern void __upc_affinity_cpu_avoid_set (const int, const upc_cpu_avoid_p);
extern int __upc_affinity_init (const upc_info_p, const upc_cpu_avoid_p,
				const char **err_msg);
extern void __upc_affinity_set (const upc_info_p, const int);
extern int __upc_affinity_supported (void);

#endif /* !_UPC_AFFINITY_H_ */
