/*	@(#)nfsstrncpy.c	UniPlus 2.1.1	*/

/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, truncating to copy n bytes
 * return ptr to null in s1 or s1 + n
 */

char *
nfsstrncpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
        register i;

        for (i = 0; i < n; i++) {
                if ((*s1++ = *s2++) == '\0') {
                        return (s1 - 1);
                }
        }
        return (s1);
}
