/*	@(#)map.h	UniPlus VVV.2.1.3	*/
/*
 *	Map Structure - used to manage contiguous space.
 */

typedef struct	map	{
	int	m_size;
	int	m_addr;
} map_t;

#define	mapstart(X)	&X[1]
#define	mapsize(X)	X[0].m_size
#define mapwant(X)	X[0].m_addr
#define	mapdata(X)	{(X)-2, 0} , {0, 0}
#define mapinit(X, Y)	X[0].m_size = (Y)-2;

#ifndef AUTOCONFIG
extern 	struct  map	sptmap[];
#else
extern 	struct  map	*sptmap;
#endif AUTOCONFIG
