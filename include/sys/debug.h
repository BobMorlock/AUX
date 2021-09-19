/*	@(#)debug.h	UniPlus VVV.2.1.3	*/
#define	YES 1
#define	NO  0

#ifndef ORIG3B20

#ifdef HOWFAR
#define ASSERT(EX) if(!(EX))assfail("EX", __FILE__, __LINE__)
#define TRACE(x,y)	if (x) debugtrace y
#ifdef AUTOCONFIG
#define AUTO_TRACE(x,y)	if (x) auto_trace y
#else
#define AUTO_TRACE(x,y)	if (x) debugtrace y
#endif AUTOCONFIG
#else
#define ASSERT(x)
#define TRACE(x,y)
#define AUTO_TRACE(x,y)
#endif

#else ORIG3B20

#if OSDEBUG == YES
#define ASSERT(EX) if(!(EX))assfail("EX", __FILE__, __LINE__)
#else
#define ASSERT(x)
#endif

#endif ORIG3B20
