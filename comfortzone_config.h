#ifndef _COMFORTZONE_CONFIG_H
#define _COMFORTZONE_CONFIG_H

#undef DEBUG

// Serial console
//#define OUTSER Serial1

#if defined(DEBUG) && defined(OUTSER)
#define DPRINT(args...)    OUTSER.print(args)
#define DPRINTLN(args...)  OUTSER.println(args)
#else
#define DPRINT(args...)
#define DPRINTLN(args...)
#endif

// normal print
#if defined(OUTSER)
#define NPRINT(args...)    OUTSER.print(args)
#define NPRINTLN(args...)  OUTSER.println(args)
#else
#define NPRINT(args...)
#define NPRINTLN(args...)
#endif


#endif
