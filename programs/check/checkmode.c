/* $Id: checkmode.c,v 1.5 2003/02/14 14:52:31 cegger Exp $
******************************************************************************

   Checkmode - Test for all available modes and output a list of them.

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

/* This is needed for the HAVE_* macros */
#include "config.h"

#include <ggi/ggi.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>  /* getopt */
#endif

ggi_visual_t vis;

/* most graphics resolutions taken from XFree */
/* EGA/CGA, text resolutions from */
/* M. Uphff: Die Programmierung der EGA/VGA Grafikkarte */
struct resolution {
	int x;
	int y;
} defaultresolutions[]= { 
	{320,200},
	{320,240},
	{320,350}, /* EGA text */
	{320,400},                 /* Brian's WD driver can do it. */
	{320,480},                 /*  " */
	{360,400}, /* VGA text */
	{400,300},
	{480,300},
	{512,384},
	{640,200}, /* EGA */
	{640,240},                 /* Brian's WD driver */
	{640,350}, /* EGA */
	{640,400},
	{640,480},
	{720,350}, /* MDA text */
	{720,400}, /* VGA text */
	{720,480}, /* NTSC, c't 98-7-72 */
	{720,576}, /* PAL , c't 98-7-72 */
	{800,600},
	{1024,768},
	{1152,864},
	{1152,900}, /* STD Sparc graphics resolution */
	{1200,1024},
	{1280,854}, /* max. resolution for Apple PowerBook with 15.2" TFT displays */
	{1600,1200},
	{0,0}          /* End mark!! */
};

ggi_graphtype defaultgraphtypes[]={
	/* can't handle textmode yet */
	GT_1BIT,                /*  1 bpp graphics              */
        GT_4BIT,                /*  4 bpp graphics              */
        GT_8BIT,                /*  8 bpp graphics              */
        GT_15BIT,               /* 15 bpp graphics              */
        GT_16BIT,               /* 16 bpp graphics              */
        GT_24BIT,               /* 24 bpp graphics              */
        GT_32BIT,               /* 24 bpp word aligned          */
        GT_INVALID
};

int setmod; /* true: do not only check, but set also */
int verbose;/* true: report all test results. False: small table */
int autof;   /* true: virtual size is GGI_AUTO, false: virtual = visible */
int xinc;   /* x increment */
int fast;   /* check only some y values for each X span, big speed increase! */
int async;  /* false: sync mode, true: async mode */

int iswrong=0; /* if the checkmode return is wrong. */

void checkoneresolution(int j, int x, int y, int xvir, int yvir )
{
	ggi_mode suggest;
	int res;
	int broken;   /* testing, if the suggested mode works */

	if (verbose){
		printf ("\n %d/%d:", GT_DEPTH(defaultgraphtypes[j]),
                        GT_SIZE(defaultgraphtypes[j]));
	}
	/*printf("Checking %d %d %d %d ", x,y,xvir,yvir);*/

	res = ggiCheckGraphMode(vis,x,y,xvir,yvir,
				defaultgraphtypes[j],
				&suggest);
	if (res==0){
		if (
			!autof && (
			(suggest.virt.x != xvir) ||
			(suggest.virt.y != yvir))
			) {
			printf("C"); 
			iswrong = 1;
                        /* has illegally changed virt but succeeded */
		} else if (setmod){
			/*ggiFlush(vis); -- segfaults on the first test */
			if (ggiSetGraphMode(vis,x,y,xvir,yvir,
					    defaultgraphtypes[j])){
				/* failed */
				iswrong = 1;
				printf("S");
			} else {
				printf("+");
			}
			/*ggiFlush(vis);  / * necessary?? */

		} else {
			printf("+");
		}
	} else {
		broken=ggiCheckMode(vis,&suggest); /* does that one work?*/
		/* it has to! */
		if (broken) {
			iswrong = 1;
		}

		if (verbose){
			printf("- (%dx%d*%dx%d %d/%d %s)",
			       suggest.visible.x,suggest.visible.y,
			       suggest.virt.x,suggest.virt.y,
			       GT_DEPTH(suggest.graphtype),
			       GT_SIZE(suggest.graphtype),
			       broken?"B":"+");
		} else {
			if (broken){
				printf("B");
			} else if (GT_DEPTH(suggest.graphtype) > 
				   GT_DEPTH(defaultgraphtypes[j])) {
				printf(">");
			} else if (GT_DEPTH(suggest.graphtype) < 
				   GT_DEPTH(defaultgraphtypes[j])) {
				printf("<");
			} else if (((suggest.visible.x < x)&&
				    (suggest.visible.y <= y))||
				   ((suggest.visible.x <= x)&&
				    (suggest.visible.y < y))){
                                printf("^");
			} else if (((suggest.visible.x > x)&&
				    (suggest.visible.y >= y))||
				   ((suggest.visible.x >= x)&&
				    (suggest.visible.y > y))){
				printf("v");
			} else if ((suggest.visible.x == x) &&
				   (suggest.visible.y == y) &&
				   ((suggest.virt.x != xvir) ||
				    (suggest.virt.y != yvir))){
				printf("*");
				iswrong = 1;
			} else {
				printf("o");
				iswrong = 1;
			}
		}
	}
	if (verbose){
		/*printf("\n");*/
	} else {
		printf("\t");
	}
}

void checkstandardresolutions(void)
{
	int i,j,x,y;

	if (!verbose){
		printf("\t\t");
		for (j=0;defaultgraphtypes[j]!=GT_INVALID;j++){
			printf("%d/%d\t",
			       GT_DEPTH(defaultgraphtypes[j]),
			       GT_SIZE(defaultgraphtypes[j]));
		}
		printf("\n");
	}
	
	i=0;j=0;
	x=defaultresolutions[i].x;
	y=defaultresolutions[i].y;
	while (x!=0){
		
		printf("%s%dx%d%s",verbose?"Checking ":"",
		       x,y,
		       verbose?" :":"  \t");
		for (j=0;defaultgraphtypes[j]!=GT_INVALID;j++){
			/*printf ("Depth %d [%s]:  ",
			  j,GRAPHTYPE(defaultgraphtypes[j]));*/
			if (autof){
				checkoneresolution(j,x,y,GGI_AUTO,GGI_AUTO);
			} else {
				checkoneresolution(j,x,y,x,y);
			}
		}
		i++;
		x=defaultresolutions[i].x;
		y=defaultresolutions[i].y;
		/*if (!verbose)*/{
			printf("\n");
		}
	}
	return;
}


struct area {
	int x1;    /* lowest x that */
	int x2;    /* highest x that */
	int xref;  /* suggests  (0=) itself or xref */
	int xvref; /* suggests  virtual (0=) itself or xvref */
	int y1;    /* lowest y that */
	int y2;    /* highest y that */
	int yref;  /* suggests  (0=) itself or yref */
	int yvref; /* suggests  virtual (0=) itself or yvref */
#if 0 /* only needed if areas are to be saved */
	struct area * nextx ; /* next area with higher x value */
	/* nextx->x1 = x2+1; nextx->y1=y1 */
	struct area * nexty ; /* next area with higher y value */
	/* nexty->x1 = x1  ; nexty->y1=y2+1 */
#endif
	int supported ; /* these modes are supported */
	int broken;     /* these modes are unsupported and broken */
	/* i.e. suggested mode does not work either */
};


void scanarea(struct area * a, 
	      int gti /* graphtype index */)
/* area with x1,y1,x2,y2 initialised: maximum scanned area! */

{
	int x,y;
	struct area right, down; /* next areas recursively to be checked */
	ggi_mode sugg;
	int fail;
	int spanfailed;

	printf("checking area %d..%d x %d..%d.\n",a->x1,a->x2,a->y1,a->y2);

	right = *a;  /*save maximum values */
	/* right will keep x2,y1,y2,  x1 will be a->x2+1; */
	/* down will keep x1, y2 and x2 will be a->x2, y1 will be a->y2+1 */
	down  = *a;

	/* define the minimum area (1x1) */
	x=a->x1;
	y=a->y1;
	a->x1=x;
	a->y1=y;
	/*a->x2=x;
	a->y2=y;*/
	fail=ggiCheckGraphMode(vis,x,y,autof?GGI_AUTO:x,autof?GGI_AUTO:y,
			      defaultgraphtypes[gti],&sugg);
	if (fail){
		if (GT_DEPTH(sugg.graphtype) !=
		    GT_DEPTH(defaultgraphtypes[gti]) ) {
			printf("Depth %d is not supported at all.\n",
			       GT_DEPTH(defaultgraphtypes[gti]));
			return;
		}
	}
	if((a->broken=ggiCheckMode(vis,&sugg))!=0){
		iswrong = 1;
	}
	a->supported=!fail;
	a->xref= sugg.visible.x;
	a->yref= sugg.visible.y;
	a->xvref= sugg.virt.x;
	a->yvref= sugg.virt.y;
	
	/* peek to the right: */
	/* it might be always this x for this area */
	/* or always accepted */
	
	fail=ggiCheckGraphMode(vis,x+xinc,y,
			       autof?GGI_AUTO:x+xinc,autof?GGI_AUTO:y,
			       defaultgraphtypes[gti],&sugg);
	if (
		(a->supported==!fail)&&
		(a->yref== sugg.visible.y)&&
		(a->yvref== sugg.virt.y)&&
		(a->broken==ggiCheckMode(vis,&sugg))
		){
		if ( (a->xref==x)&&(sugg.visible.x== x+xinc) ){
			a->xref= 0;  /* let's try: 'stays the same' */
		}
		if ( (a->xvref==x)&&(sugg.virt.x== x+xinc) ){
			a->xvref= 0;  /* let's try: 'stays the same' */
		}
	}
	/*printf("peeking right: vis.%d virt.%d.\n",a->xref,a->xvref);*/
	/* peek down: */
	/* it might be always this y for this area */
	/* or allways accepted */
	
	fail=ggiCheckGraphMode(vis,x,y+1,
			       autof?GGI_AUTO:x,autof?GGI_AUTO:y+1,
			       defaultgraphtypes[gti],&sugg);
	if (
		(a->supported==!fail)&&
		( (a->xref== sugg.visible.x) || (a->xref==0)) &&
		( (a->xvref== sugg.virt.x)   || (a->xref==0)) &&
		(a->broken==ggiCheckMode(vis,&sugg))
		){
		/*printf("y: same base\n");*/
		if ( (a->yref==y)&&(sugg.visible.y== y+1) ){
			a->yref= 0;  /* let's try: 'stays the same' */
		}
		if ( (a->yvref==y)&&(sugg.virt.y== y+1) ){
			a->yvref= 0;  /* let's try: 'stays the same' */
		}
	}
	/*printf("peeking down: vis.%d virt.%d.\n",a->yref,a->yvref);*/
	
	/*printf("pling %d %d\n",a->yref,a->yvref);*/
	/* extend it to higher y */
	y++;
	if (y > a->y2) 
		return;

	fail=ggiCheckGraphMode(vis,x,y,autof?GGI_AUTO:x,autof?GGI_AUTO:y,
			       defaultgraphtypes[gti],&sugg);


#define CHECKACCEPTL(l,m,n) ( (printf(#m"=%d ",m),m==0) ? (\
printf("%d =? %d ",l,n), l==n) : (printf("%d =? %d ",l,m), l==m))
#define CHECKACCEPT(l,m,n) ( (m==0) ? (l==n) : (l==m))
	
	while (	(a->supported==!fail)&&
		CHECKACCEPT(sugg.visible.x,a->xref,x) &&
		CHECKACCEPT(sugg.visible.y,a->yref,y) &&
		CHECKACCEPT(sugg.virt.x,a->xvref,x) &&
		CHECKACCEPT(sugg.virt.y,a->yvref,y) &&

		(a->broken==ggiCheckMode(vis,&sugg)) && (y <= a->y2)){
		/*printf("%d succeeded\n",y);*/
		y++;
		fail=ggiCheckGraphMode(vis,x,y,autof?GGI_AUTO:x,autof?GGI_AUTO:y,
				       defaultgraphtypes[gti],&sugg);

	}
#if 0
	printf("%dx%d is different:\n",x,y);
	printf("%d %d\n",a->supported,!fail);
	printf("%d %d %d\n",sugg.virt.x,x,
	       CHECKACCEPT(sugg.visible.x,a->xref,x));
	printf("%d %d %d\n",sugg.virt.y,y,
		CHECKACCEPT(sugg.visible.y,a->yref,y));
	printf("%d %d %d\n",sugg.visible.x,x,
		CHECKACCEPT(sugg.virt.x,a->xvref,x));
	printf("%d %d %d\n",sugg.visible.y,y,
		CHECKACCEPT(sugg.virt.x,a->yvref,y));
	printf("%d %d\n",a->broken,ggiCheckMode(vis,&sugg));
#endif
	y--;
	a->y2=y;
	down.y1=y+1;
	/* now the first span is tested. */
	/* go to the right. */

	/*printf("Testing span x= :");*/
	spanfailed=0; /*if I hit a difference once, this span is 'out'*/
	while (!spanfailed){
		x+=xinc;
		/*printf("%4d",x);*/
		if (x> a->x2){
			spanfailed=1;
			/*printf("x>a->x2.\n");*/
		}
		for (y=a->y1; (y<a->y2)&& !spanfailed ;
		     y+= (fast && ((a->y2 - a->y1)>10))?
			     (a->y2 - a->y1)/10
			     :1){
			
			/*printf(" y=%d ;",y);*/
			fail=ggiCheckGraphMode(
				vis,x,y,
				autof?GGI_AUTO:x,autof?GGI_AUTO:y,
				defaultgraphtypes[gti],&sugg);
			spanfailed= /*spanfailed ||*/ !(
				(a->supported==!fail)&&
				CHECKACCEPT(sugg.visible.x,a->xref,x) &&
				CHECKACCEPT(sugg.visible.y,a->yref,y) &&
				CHECKACCEPT(sugg.virt.x,a->xvref,x) &&
				CHECKACCEPT(sugg.virt.y,a->yvref,y) &&
				
				(a->broken==ggiCheckMode(vis,&sugg)));
#if 0
			if (spanfailed){
	printf("%dx%d is different:\n",x,y);
	printf("%d %d\n",a->supported,!fail);
	printf("virt x %d %d is %d\n",sugg.virt.x,x,
	       CHECKACCEPT(sugg.visible.x,a->xref,x));
	printf("virt y %d %d is %d\n",sugg.virt.y,y,
		CHECKACCEPT(sugg.visible.y,a->yref,y));
	printf("vis x %d %d is %d\n",sugg.visible.x,x,
		CHECKACCEPT(sugg.virt.x,a->xvref,x));
	printf("%d %d %d\n",sugg.visible.y,y,
		CHECKACCEPT(sugg.virt.x,a->yvref,y));
	printf("%d %d\n",a->broken,ggiCheckMode(vis,&sugg));
			}
#endif

		}
	}
	x -= xinc;
	
	a->x2=x;
	right.x1=x+xinc;
	down.x2=x;

       
#define EQORVAR(x) if (x==0) printf(" == "); else printf("%d ",x)

	printf("Area %d..%dx%d..%d : ",
	       a->x1,a->x2,a->y1,a->y2);
	if (a->supported){
		printf("supported\n");
	} else {
		printf("%s (sugg: ",a->broken?"broken":"failed");
		EQORVAR(a->xref);
		printf("x");
		EQORVAR(a->yref);
		printf("#");
		EQORVAR(a->xvref);
		printf("x");
		EQORVAR(a->yvref);
		printf(")\n");
	}

	if (down.y2>=down.y1)
		scanarea(&down,gti);
	if (right.x2>=right.x1)
		scanarea(&right,gti);
}

	


void checkallresolutions(void)
{
	int j;
	struct area a;

	const int xmin=8;
	const int xmax=/*48 */ 2048 ;
	const int ymin=8;
	const int ymax=/* 9 */ 2048 ;

	for (j=0;defaultgraphtypes[j]!=GT_INVALID;j++){
		printf (" %d/%d:  ",
			GT_DEPTH(defaultgraphtypes[j]),
			GT_SIZE(defaultgraphtypes[j]));
		a.x1=xmin;
		a.y1=ymin;
		a.x2=xmax;
		a.y2=ymax;
		scanarea(&a,j);
	}
}
		
void usage(char * s)
{
	printf("Usage: %s [options]\n"
	       "     available options:\n"
	       "     -s: set mode (not only check mode)\n"
	       "     -v: verbose\n"
	       "     -a: give a virtual size of GGI_AUTO (instead of virt=vis)\n"
		   "     -A: use asynchronous mode\n"
	       "     -l: long (area) test, not only the standard values\n"
	       "       -4: test only x divisible by 4\n"
	       "       -8: test only x divisible by 8\n"
	       "       -f: fast: test only some values in the area test\n"
	     /*  "     planned options:\n"
	       "     (none)\n"*/
	       , s);
	printf("The verbose output includes for each failing mode the\n"
	       "suggested mode, the short output gives a table containig\n"
	       " + for possible modes\n"
		   " C for success, but the virtual size changed (which is not allowed)\n"
	       " B for impossible modes, when the suggested mode doesn't work"
	       " either (broken),\n"
	       " > for 'higher' graphtypes (=bitdepth) suggested,\n"
	       " < for lower graphtypes\n"
	       " ^ for lower x AND y resolution\n"
	       " v for higher x AND y resolution\n"
	       " * for different virtual size (broken GGI_AUTO handling?? )\n"
	       " o failing for other reasons.\n");
	printf(" S checking the mode succeeded, but setting it failed.\n"
	       "   (note that the original mode is set, not the suggested one.)\n");
	printf("$Id: checkmode.c,v 1.5 2003/02/14 14:52:31 cegger Exp $\n");
	exit(0);
}

int main(int argc, char **argv)
{
#ifdef HAVE_GETOPT
	char * myself = argv[0];
	int op;
#endif
	int longtest=0;

	setmod=0;
	verbose=0;
	autof=0;
	xinc=1;
	fast=0;
	async=0;

#ifdef HAVE_GETOPT
	while ( (op=getopt(argc,argv,"48ahsvflA"))!=EOF ){
		switch(op){
		case 's': /*fprintf(stderr,"Setting modes also");*/ 
			setmod=1;
			break;
		case 'v': 
			verbose=1;
			break;
		case 'a':
			autof=1;
			break;
		case 'A':
			async=1;
			break;
		case 'f':
			fast=1;
			break;
		case '4':
			xinc=4;
			break;
		case '8':
			xinc=8;
			break;
		case 'l':
			longtest=1;
			break;
		case 'h': usage(myself); /* never return */ break;

		default: printf("Unknown option %c\n",op);
			usage(myself);
		}
	}
#endif
	if (longtest==0){
		if (xinc==4) printf("option -4 ignored (use only with -l).\n");
		if (xinc==8) printf("option -8 ignored (use only with -l).\n");
		if (fast==1) printf("option -f ignored (use only with -l).\n");
	}
	
	/*usage(myself);*/
	
	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}
	if ((vis=ggiOpen(NULL)) == NULL) {
		fprintf(stderr,
			"%s: unable to open default visual, exiting.\n",
			argv[0]);
		exit(1);
	}

	if (async)
		ggiSetFlags(vis,GGIFLAG_ASYNC);

	checkstandardresolutions();

	
	if (longtest){
		checkallresolutions();
	}

        ggiClose(vis);

        ggiExit();

	if (iswrong){
		printf("At least for one resolution ggiCheckMode is wrong.\n");
	} else {
		printf("ggiCheckMode seems correct for all tested modes.\n");
	}

	return (0);
}




