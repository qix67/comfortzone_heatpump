#ifndef _COMFORTZONE_CONFIG_H
#define _COMFORTZONE_CONFIG_H

#undef DEBUG

// Serial console
#define OUTSER Serial

#if defined(DEBUG)
#define DPRINT(args...)    OUTSER.print(args)
#define DPRINTLN(args...)  OUTSER.println(args)
#else
#define DPRINT(args...)
#define DPRINTLN(args...)
#endif

// normal print
#define NPRINT(args...)    OUTSER.print(args)
#define NPRINTLN(args...)  OUTSER.println(args)

#endif
