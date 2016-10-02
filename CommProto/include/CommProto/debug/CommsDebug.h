/*
  Standard CommsLib debug services.

  Copyright (C) 2016  Michael Wallace, Kartik Soni, Mario Garcia.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __COMMS_DEBUG_H
#define __COMMS_DEBUG_H

#include <stdio.h>
#include <stdarg.h>

// Controls logging information. Set this to 1, if you wish to 
// display debug information from comms_debug_log.
#if defined(_DEBUG) && _DEBUG
 #define __COMMS_DEBUG_LOG 1
#else
 #define __COMMS_DEBUG_LOG 0
#endif

#if __COMMS_DEBUG_LOG
 #define __COMMS_DEBUG_PRINT
 #define comms_debug_log(debug) printf("%s\n", debug)
#else
 #define comms_debug_log(debug)
#endif // __COMMS_DEBUG

#if defined(__COMMS_DEBUG_PRINT)
 #define COMMS_DEBUG(debug, ...) printf(debug , ##__VA_ARGS__)
#else
 #define COMMS_DEBUG(debug, ...)
#endif // __COMMS_DEBUG_PRINT

#ifdef __COMMS_DEBUG_FATAL_EXIT
 #ifdef __COMMS_DEBUG_LOG
  #define comms_fatal(messg, call) { \
            comms_debug_log("FATAL: "); \
            comms_debug_log("%s", messg); \
	    call = -1; \
          }
 #else
  #define comms_fatal(messg, call)
 #endif // __COMMS_DEBUG_FATAL_EXIT
#else
 #define comms_fatal(messg, call)
#endif // __COMMS_DEBUG_LOG

namespace comnet {
namespace debug {

} // Console namespace
} // Comnet namespace

#endif // __COMMS_DEBUG_H 
