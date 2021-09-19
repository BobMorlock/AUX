/*	@(#)geninit.c	UniPlus 2.1.2	*/
#include	<sys/param.h>

main()
{
	printf("/* Due to a BUG in the generic linker\n");
	printf("   the size + length must not add up to \n");
	printf("   something with bit 31 set */\n");
	printf("MEMORY {\n");
	printf("	VALID: org = 0x%X, len = 0x%X\n", KMEMORG,
					0x7fffffff-KMEMORG);
	printf("}\n");
	printf("SECTIONS {\n");
#ifdef AUTOCONFIG
	printf("	pstart 0x%X: {\n", KMEMORG);
	printf("			%s (.text, .data)\n",PFILE);
	printf("		}\n");
	printf("	.text 0x%X: {}\n", TEXTSTART);
	printf("	.data 0x%X: {}\n", DATASTART);
	printf("	.bss  0x%X: {}\n", BSSSTART);
	printf("	MODULES 0x0(COPY): {}\n");
#else
	printf("	.text: {}\n");
	printf("	GROUP: {\n");
	printf("		.data: {}\n");
	printf("		.bss: {}\n");
	printf("	}\n");
#endif AUTOCONFIG
	printf("}\n");
	printf("u = 0x%X;\n", UDOT);

	exit(0);
}
