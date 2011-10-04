/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2011 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sc_iostream.h - Portable iostream header file wrapper.

  Original Author: Martin Janssen, Synopsys, Inc.

  CHANGE LOG AT END OF FILE
 *****************************************************************************/

#ifndef SC_IOSTREAM_H
#define SC_IOSTREAM_H

#if !defined( _MSC_VER ) && /*!defined( __HP_aCC ) &&*/ !defined( __BORLANDC__ )

#   include <iostream>

#   if defined(__GNUC__) && (__GNUC__ == 2) // Maintain compatibility for 2.95.x
#   	include <strstream>
#   else
#   	include <sstream>
#   endif

#   include <fstream>
#   include <cstddef>
#   include <cstring>

#else
#if defined( _MSC_VER )
    // VC++6 with standard library

#  include <iostream>
#  include <strstream>
    
#  include <fstream>

#  include <cstddef>

#else
/*
#if defined( __HP_aCC )
    // HP aCC

#   include <iostream.h>
#   include <strstream.h>
#   include <fstream.h>

#endif // HP
*/
#endif // MSC
#endif // Everyone else


// shortcuts that save some typing

#ifdef CCAST
#    undef CCAST
#endif
#define CCAST       const_cast

#ifdef DCAST
#    undef DCAST
#endif
#define DCAST     dynamic_cast

#ifdef RCAST
#    undef RCAST
#endif
#define RCAST reinterpret_cast

#ifdef SCAST
#    undef SCAST
#endif
#define SCAST      static_cast

// $Log: sc_iostream.h,v $
// Revision 1.3  2011/08/26 20:46:18  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.2  2011/02/18 20:38:43  acg
//  Andy Goodrich: Updated Copyright notice.
//
// Revision 1.1.1.1  2006/12/15 20:20:06  acg
// SystemC 2.3
//
// Revision 1.3  2006/01/13 18:53:10  acg
// Andy Goodrich: Added $Log command so that CVS comments are reproduced in
// the source.
//

#endif // !defined(SC_IOSTREAM_H)