#ifndef _COMFORTZONE_CONFIG_H
#define _COMFORTZONE_CONFIG_H

#define HP_PROTOCOL_1_6 160
#define HP_PROTOCOL_1_8 180

// adjust HP_PROTOCOL value according to protocol version to use
#define HP_PROTOCOL HP_PROTOCOL_1_6

#undef DEBUG
// uncomment to enable debug mode
//#define DEBUG

// Serial console
// uncomment and set to serial port where debug data should be sent
//#define OUTSER Serial

#define COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE 256

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
