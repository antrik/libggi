/* $Id: cube3d.c,v 1.19 2005/07/30 08:43:02 soyt Exp $
******************************************************************************

   cube3d.c - display up top 6 other LibGGI applications on the sides of
   	      a spinning 3D cube. See the man page for more info.

   Authors:	1998-1999 Andreas Beck		[becka@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

/* For autoconf inline support */
#include "config.h"

/* Include the LibGGI declarations.
 */
#include <ggi/gii.h>
#include <ggi/ggi.h>

/* Include the necessary headers used for e.g. error-reporting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

/* A little math for the 3d stuff not much ...
 */
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* We do shm here !
 */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_SHM_H

#include <sys/shm.h>

#elif defined(HAVE_WINDOWS_H)

/* Very rudimentary mapping from unix shm api to win32 file mapping api */

#include <windows.h>

#define shmget(key, size, shmflg) \
    (int)CreateFileMapping( \
	INVALID_HANDLE_VALUE, \
	NULL, \
	PAGE_READWRITE | SEC_COMMIT, \
	0, /* size not larger than 2^32, I hope. */ \
	size, \
	key)
#define shmat(shmid, shmaddr, shmflg) \
	MapViewOfFile((HANDLE)shmid, FILE_MAP_WRITE, 0, 0, 0)
#define shmdt(shmaddr) \
	UnmapViewOfFile(shmaddr)
#define shmctl(shmid, cmd, buf) \
	CloseHandle((HANDLE)shmid)

#endif
#include <signal.h>

static ggi_visual_t vis;
static int vissizex, vissizey;

/* Pixel value for white, red and black. See main() on how to get at it.
 */
static ggi_pixel white;
static ggi_pixel red;
static ggi_pixel black;

/* In case we were called with wrong parameters, give an explanation.
 */
static void usage(const char *prog)
{
	fprintf(stderr, "Usage:\n\n"
		"%s [[-slavex,slavey] [program]]* \n\n"
		"Example: %s -320,200 nixterm -160,100 ./demo", prog,
		prog);
	exit(EXIT_FAILURE);
}

typedef double Matrix3D[3][3];
typedef double Vector3D[3];
typedef double Vector2D[2];

typedef struct {
	Vector3D origin;
	Vector2D projected;
} Point3D;

typedef struct Texture {
	int filex, filey;
	ggi_visual_t vis;
	pid_t pid;
	 ggi_pixel(*mapper) (int posx, int posy, struct Texture * file);
	void *plb;
} Texture;


static Vector2D the_texcoord[4] = {
	{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}
};

static Texture the_textures[6];

static Point3D the_points[24];

static Point3D the_original_points[24] = {
	{{-100, -75, -130}, {0, 0}},
	{{100, -75, -130}, {0, 0}},
	{{100, +75, -130}, {0, 0}},
	{{-100, +75, -130}, {0, 0}},

	{{-130, -75, 100}, {0, 0}},
	{{-130, -75, -100}, {0, 0}},
	{{-130, +75, -100}, {0, 0}},
	{{-130, +75, 100}, {0, 0}},

	{{100, -75, 130}, {0, 0}},
	{{-100, -75, 130}, {0, 0}},
	{{-100, +75, 130}, {0, 0}},
	{{100, +75, 130}, {0, 0}},

	{{130, -75, -100}, {0, 0}},
	{{130, -75, 100}, {0, 0}},
	{{130, +75, 100}, {0, 0}},
	{{130, +75, -100}, {0, 0}},

	{{-100, -105, +75}, {0, 0}},
	{{100, -105, +75}, {0, 0}},
	{{100, -105, -75}, {0, 0}},
	{{-100, -105, -75}, {0, 0}},


	{{-100, 105, -75}, {0, 0}},
	{{100, 105, -75}, {0, 0}},
	{{100, 105, +75}, {0, 0}},
	{{-100, 105, +75}, {0, 0}}
};

static int numpoints = 24;

typedef struct {
	int numedges;
	Point3D *points;
	Vector2D *texcoord;
	Vector3D normal, middle;
	double visible;
	Texture *texture;
} Polygon3D;

static Polygon3D the_polys[6] = {
	{4, the_points + 0, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 0},
	{4, the_points + 4, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 1},
	{4, the_points + 8, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 2},
	{4, the_points + 12, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 3},
	{4, the_points + 16, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 4},
	{4, the_points + 20, the_texcoord, {0, 0, 0}, {0, 0, 0}, 0,
	 the_textures + 5},
};

typedef struct {
	int polynum;
	double zmiddle;
} zorder3D;

typedef struct {
	int numpolys;
	Polygon3D *polys;
} Scene3D;

static Scene3D the_scene = {
	6,
	the_polys
};

static Vector3D eyepos = { 0, 0, -1000 };

static inline void
Matrix_times_Vector(double *mat, double *vec, double *result)
{
	int x, y;
	for (x = 0; x < 3; x++) {
		result[x] = 0.0;
		for (y = 0; y < 3; y++) {
			result[x] += mat[x * 3 + y] * vec[y];
		}
	}
}

static inline void Matrix_times_Matrix(double *mat, double *mat2)
{
	int x, y, z;
	double result[9];
	for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			result[x + 3 * y] = 0.0;
			for (z = 0; z < 3; z++)
				result[x + 3 * y] +=
				    mat2[z + 3 * y] * mat[x + 3 * z];
		}
	}
	memcpy(mat, result, sizeof(result));
}

static inline double Matrix_Deter(double *mat)
{
	return mat[0 + 3 * 0] * mat[1 + 3 * 1] * mat[2 + 3 * 2]
	    + mat[1 + 3 * 0] * mat[2 + 3 * 1] * mat[0 + 3 * 2]
	    + mat[2 + 3 * 0] * mat[0 + 3 * 1] * mat[1 + 3 * 2]
	    - mat[2 + 3 * 0] * mat[1 + 3 * 1] * mat[0 + 3 * 2]
	    - mat[1 + 3 * 0] * mat[0 + 3 * 1] * mat[2 + 3 * 2]
	    - mat[0 + 3 * 0] * mat[2 + 3 * 1] * mat[1 + 3 * 2];
}

static inline double Vector_Length(double *vec)
{
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

static inline void Vector_VecProd(double *in1, double *in2, double *out)
{
	out[0] = in1[1] * in2[2] - in1[2] * in2[1];
	out[1] = in1[2] * in2[0] - in1[0] * in2[2];
	out[2] = in1[0] * in2[1] - in1[1] * in2[0];
}

static inline double Vector_ScalarProd(double *in1, double *in2)
{
	return in1[0] * in2[0] + in1[1] * in2[1] + in1[2] * in2[2];
}

static inline void Vector_Subtract(double *in1, double *in2, double *out)
{
	out[0] = in1[0] - in2[0];
	out[1] = in1[1] - in2[1];
	out[2] = in1[2] - in2[2];
}

static inline void Vector_Add(double *in1, double *in2, double *out)
{
	out[0] = in1[0] + in2[0];
	out[1] = in1[1] + in2[1];
	out[2] = in1[2] + in2[2];
}

static inline void Vector_Scale(double scale, double *in1, double *out)
{
	out[0] = in1[0] * scale;
	out[1] = in1[1] * scale;
	out[2] = in1[2] * scale;
}

static inline void transform_point(double *mat, Point3D * in,
				   Point3D * out)
{
	Matrix_times_Vector(mat, in->origin, out->origin);
	out->projected[0] = out->origin[0] * (out->origin[2] + eyepos[2]) / eyepos[2] + vissizex / 2;	/* Fixme - perspective */
	out->projected[1] =
	    out->origin[1] * (out->origin[2] + eyepos[2]) / eyepos[2] +
	    vissizey / 2;
}

static inline void normvec_poly(Polygon3D * poly)
{
	Vector3D v1, v2;
	Vector_Subtract(poly->points[1].origin, poly->points[0].origin,
			v1);
	Vector_Subtract(poly->points[2].origin, poly->points[1].origin,
			v2);
	Vector_VecProd(v1, v2, poly->normal);
}

static inline void visible_poly(Polygon3D * poly)
{
	Vector3D view;
	Vector_Subtract(poly->middle, eyepos, view);
	poly->visible = Vector_ScalarProd(view, poly->normal);
}

static inline void midvec_poly(Polygon3D * poly)
{
	int x;
	poly->middle[0] = poly->middle[1] = poly->middle[2] = 0.0;
	x = poly->numedges;
	while (x--) {
		Vector_Add(poly->middle, poly->points[x].origin,
			   poly->middle);
	}
	Vector_Scale(1.0 / poly->numedges, poly->middle, poly->middle);
}

static Matrix3D current = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};

static Matrix3D speed = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0}
};

static ggi_pixel PatRemap(int posx, int posy, Texture * file)
{
	ggi_color hlpmap;
	ggi_pixel pixel;

	ggiGetPixel(file->vis, posx, posy, &pixel);
	ggiUnmapPixel(file->vis, pixel, &hlpmap);
	return ggiMapColor(vis, &hlpmap);
}

static ggi_pixel PatNomap(int posx, int posy, Texture * file)
{
	ggi_pixel pixel;

	ggiGetPixel(file->vis, posx, posy, &pixel);
	return pixel;
}

static ggi_pixel Plb8(int posx, int posy, Texture * file)
{
	return (ggi_pixel) * ((uint8 *) file->plb + posx +
			      posy * file->filex);
}

static ggi_pixel Plb16(int posx, int posy, Texture * file)
{
	return (ggi_pixel) * ((uint16 *) file->plb + posx +
			      posy * file->filex);
}

static ggi_pixel Plb32(int posx, int posy, Texture * file)
{
	return (ggi_pixel) * ((uint32 *) file->plb + posx +
			      posy * file->filex);
}

#define FIX_SHIFT	(16)
#define FIX_ZERO		(0x00000000)
#define FIX_HALF		(0x00008000)

/* Fourth try. Use scanline technique
 */
static void doblit(Polygon3D * poly, int transp)
{
	int miny, maxy, x, lastx, y;
	double xx, patxadd, patyadd;
	ggi_pixel pixel;

	struct {
		int xpos, patx, paty;
	} min, max, hlp;

	memset(&min, 0, sizeof(min));
	memset(&max, 0, sizeof(max));
	miny = INT_MIN;
	maxy = INT_MAX;

	/* Calc min/max scanline */
	x = poly->numedges;
	while (x--) {
		if (poly->points[x].projected[1] < miny)
			miny = poly->points[x].projected[1];
		if (poly->points[x].projected[1] > maxy)
			maxy = poly->points[x].projected[1];
	}
	if (miny < 0)
		miny = 0;
	if (maxy >= vissizey)
		maxy = vissizey - 1;

	ggiSetGCForeground(vis, white);
	for (y = miny; y <= maxy; y++) {
		double dy1, dy2;
		min.xpos = 1000000000;
		max.xpos = -1000000000;

		x = poly->numedges;
		lastx = 0;
		while (x--) {

			/* Check if we intersect an edge with that scanline 
			 */
			dy1 = poly->points[x].projected[1] - y;
			dy2 = poly->points[lastx].projected[1] - y;
			if ((dy1 > 0.0 && dy2 <= 0.0) ||
			    (dy1 < 0.0 && dy2 >= 0.0)) {

#define MKPATX \
	(hlp.patx=((int)(poly->texture->filex*		\
			(poly->texcoord[    x][0]*dy2-	\
			 poly->texcoord[lastx][0]*dy1)/	\
			(dy2-dy1)))<<FIX_SHIFT)
#define MKPATY \
	(hlp.paty=((int)(poly->texture->filey*		\
			(poly->texcoord[    x][1]*dy2-	\
			 poly->texcoord[lastx][1]*dy1)/	\
			(dy2-dy1)))<<FIX_SHIFT)

#define MKXPOS \
	hlp.xpos=((int)((poly->points[    x].projected[0]*dy2-	\
			 poly->points[lastx].projected[0]*dy1)/	\
			(dy2-dy1)))

				MKXPOS;
				if (hlp.xpos < min.xpos) {
					MKPATX;
					MKPATY;
					min = hlp;
					/* Avoid double calculation: */
					if (hlp.xpos > max.xpos)
						max = hlp;
				} else if (hlp.xpos > max.xpos) {
					MKPATX;
					MKPATY;
					max = hlp;
				}
			}
			lastx = x;
		}
#undef MKPATX
#undef MKPATY
#undef MKXPOS

		if (max.xpos < 0)
			continue;
		if (min.xpos >= vissizex)
			continue;

		if (min.xpos == max.xpos) {
			if ((pixel =
			     poly->texture->mapper((min.patx >> FIX_SHIFT),
						   (min.paty >> FIX_SHIFT),
						   poly->texture))
			    || !transp)
				ggiPutPixel(vis, min.xpos, y, pixel);
		} else {
			patxadd = (max.patx - min.patx) / (xx =
							   (max.xpos -
							    min.xpos + 1));
			patyadd = (max.paty - min.paty) / xx;
			min.patx += FIX_HALF;
			min.paty += FIX_HALF;
			for (x = min.xpos; x <= max.xpos; x++,
			     min.patx += patxadd, min.paty += patyadd) {
				if (x < 0)
					continue;
				if (x > vissizex)
					break;
				if ((pixel =
				     poly->texture->
				     mapper((min.patx >> FIX_SHIFT),
					    (min.paty >> FIX_SHIFT),
					    poly->texture)) || !transp)
					ggiPutPixel(vis, x, y, pixel);
			}
		}
	}
}

static inline void turn_add(int ax1, int ax2, double degree)
{
	Matrix3D turnit;
	memset(turnit, 0, sizeof(turnit));

	turnit[0][0] = turnit[1][1] = turnit[2][2] = 1.0;
	turnit[ax1][ax1] = turnit[ax2][ax2] = cos(degree);
	turnit[ax1][ax2] = -(turnit[ax2][ax1] = sin(degree));

	Matrix_times_Matrix((double *) speed, (double *) turnit);
}

static inline void scale(double fac)
{
	int x;
	for (x = 0; x < 9; x++)
		((double *) speed)[x] *= fac;
}

static inline void stop_speed(void)
{
	memset(speed, 0, sizeof(speed));
	speed[0][0] = speed[1][1] = speed[2][2] = 1.0;
}

static int align(Polygon3D * poly)
{
	double biggest;
	int rc;
	Vector3D help;

	stop_speed();
	rc = 1;

	biggest = Vector_Length(poly->normal);
	if (biggest <= 1e-3)
		biggest = 1e-3;
	Vector_Scale(1.0 / biggest, poly->normal, help);

	if (fabs(help[1]) > 1e-3) {
		turn_add(1, 2, 0.5 * help[1]);
		rc = 0;
	}
	if (fabs(help[0]) > 1e-3) {
		turn_add(0, 2, 0.5 * help[0]);
		rc = 0;
	}
	if (rc && help[2] < 0) {
		turn_add(1, 2, M_PI / 90);
		rc = 0;
	}

	Vector_Subtract(poly->points[1].origin, poly->points[0].origin,
			help);
	biggest = Vector_Length(help);
	if (biggest <= 1e-3)
		biggest = 1e-3;
	Vector_Scale(1.0 / biggest, help, help);
	if (fabs(help[1]) > 1e-3) {
		turn_add(1, 0, 0.5 * help[1]);
		rc = 0;
	} else if (help[0] < 0) {
		turn_add(1, 0, M_PI / 90);
		rc = 0;
	}
	return rc;
}

static void highlight_face(Polygon3D * poly)
{
	int x, lastx;
	x = poly->numedges;
	lastx = 0;
	while (x--) {
		ggiDrawLine(vis,
			    (int) poly->points[x].projected[0],
			    (int) poly->points[x].projected[1],
			    (int) poly->points[lastx].projected[0],
			    (int) poly->points[lastx].projected[1]);
		lastx = x;
	}
}

static int spawn_bg(char *what)
{
#ifdef HAVE_FORK
	int pid;

	pid = fork();
	if (pid == -1)
		return pid;
	if (pid == 0) {
		execlp("/bin/sh", "/bin/sh", "-c", what, (void *) NULL);
		exit(127);
	}
	return pid;
#elif defined(HAVE_WINDOWS_H)
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	si.cb = sizeof(si);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = 0;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;

	if(CreateProcess(
		what,
		NULL,
		NULL,
		NULL,
		TRUE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&si,
		&pi))
		return (int)pi.hProcess;
	else {
		printf("CreateProcess(\"%s\") failed\n", what);
		fflush(stdout);
		return 0;
	}
#endif
}

#if defined(HAVE_SYS_SHM_H)
#elif defined(HAVE_WINDOWS_H)
static const char *ftok(const char *pathname, int id)
{
	static char object[MAX_PATH+50];
	char *ptr;
	sprintf(object, "ggi-display-memory-shm:%s:%d", pathname, id);
	ptr = object;
	while((ptr = strchr(ptr, '\\')) != NULL)
		*ptr++ = '/';
	return object;
}
#endif

static void CheckDB(Texture * tex)
{
	int maxdb, x;
	const ggi_directbuffer *buf;
	maxdb = ggiDBGetNumBuffers(tex->vis);
	for (x = 0; x < maxdb; x++) {
		if (NULL == (buf = ggiDBGetBuffer(tex->vis, x)))
			break;
		if (buf->type & GGI_DB_SIMPLE_PLB) {
			switch (buf->buffer.plb.pixelformat->size) {
			case 8:
				tex->plb = buf->read;
				tex->mapper = Plb8;
				break;
			case 16:
				tex->plb = buf->read;
				tex->mapper = Plb16;
				break;
			case 32:
				tex->plb = buf->read;
				tex->mapper = Plb32;
				break;
			}
		}
	}
}

int main(int argc, char **argv)
{
	/* First we define a bunch of variables we will access throughout the
	 * main() function. Most of them are pretty meaningless loop-counters
	 * and helper-variables.
	 */

	const char *prog;	/* Make an alias for the program name */
	ggi_color map;		/* Helper for color mapping */

	ggi_graphtype type;	/* graphics type */

	int err, x;		/* counters and other helpers. */

	int frame = -1;		/* The frame we are drawing on. */
	int frcnt = 0;		/* framecounter for profiling performace */
	clock_t clk;		/* clock helper for profiling */

	int noquit;		/* main loop termination variable */
	int is_escape;		/* are we in escaped state ? */
	struct timeval tv;	/* timeout for polling the keyboard */

	int active_face = 0;	/* program state. Active face of the cube */
	int autoactive = 0;	/* automatic activation of "frontmost" face */
	int doalign = 0;	/* autoalignment in progress */
	int backfaces = 1;	/* show back faces */
	int blink = 0;		/* make the active frame blink */
	int transparency = 1;	/* make color 0 transparent */
	int progarg = 0;
	int slavesizex, slavesizey;
	zorder3D *zorder;

	/* The mode we want. 2 frames for doublebuffering */
	ggi_mode mode = {	/* This will cause the default mode to be set */
		2,		/* 2 frames */
		{GGI_AUTO, GGI_AUTO},	/* Default size */
		{GGI_AUTO, GGI_AUTO},	/* Virtual */
		{0, 0},		/* size in mm don't care */
		GT_AUTO,	/* Mode */
		{GGI_AUTO, GGI_AUTO}	/* Font size */
	};

	/* The memvisuals for the other programs ... */
	ggi_visual_t memvis[6];
	ggi_mode submode[6];
	int shmid[6];

	/* Get the arguments from the command line. 
	 * Set defaults for optional arguments.
	 */

	prog = *argv;
	argv++;
	argc--;
	if (strrchr(prog, '/') != NULL)
		prog = strrchr(prog, '/') + 1;

	if ((argc > 0) &&
	    ((strcmp(*argv, "-h") == 0) ||
	     (strcmp(*argv, "--help") == 0))) {

		usage(prog);	/* This is not an allowed call format. */
		return 1;	/* Tell the user how to call and fail. */
	}


	/* Allocate Z order array.
	 */
	if (NULL ==
	    (zorder = malloc(sizeof(zorder3D) * the_scene.numpolys))) {
		fprintf(stderr, "unable to allocate zorder buffer.\n");
		exit(1);
	}

	/* Open up GGI and a visual.
	 */
	if (ggiInit() != 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	vis = ggiOpen(NULL);

	if (vis == NULL) {
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		ggiExit();
		exit(1);
	}

	/* Go to async-mode.
	 */
	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Check and set the mode ...
	 */
	ggiCheckMode(vis, &mode);
	fprintf(stderr, "Suggested mode ");
	ggiFPrintMode(stderr, &mode);
	fprintf(stderr, "\n");
	err = ggiSetMode(vis, &mode);	/* now try it. it *should* work! */

	if (err) {
		fprintf(stderr, "Can't set mode\n");
		ggiClose(vis);
		ggiExit();
		return 2;
	}
	if (mode.frames > 1)
		frame = 0;	/* We can frameflip. */

	type = mode.graphtype;
	vissizex = mode.visible.x;
	vissizey = mode.visible.y;
	slavesizex = mode.visible.x / 2;
	slavesizey = mode.visible.y / 2;

	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
		ggiSetColorfulPalette(vis);
	}

	map.r = map.g = map.b = 0xFFFF;
	white = ggiMapColor(vis, &map);
	map.r = map.g = map.b = 0x0;
	black = ggiMapColor(vis, &map);
	map.g = map.b = 0x0;
	map.r = 0xFFFF;
	red = ggiMapColor(vis, &map);

	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);
	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);
	ggiPuts(vis, 0, 0, "Hold on - creating faces.");
	ggiFlush(vis);

	printf
	    ("Server is running - start another GGI demo (e.g. flying_ggis)\n");
	printf("with the following variable settings:\n");

	for (x = 0; x < 6; x++) {
		char text[1024];
		char envtext[1024];
		size_t memlen;

		/* First check for arguments.
		 */
		if (progarg < argc && argv[progarg][0] == '-') {
			/* We have an option */
			sscanf(argv[progarg] + 1, "%dx%d", &slavesizex,
			       &slavesizey);
			progarg++;
		}
		submode[x] = mode;
		submode[x].frames = 1;
		submode[x].visible.x = slavesizex;
		submode[x].visible.y = slavesizey;
		submode[x].virt.x = slavesizex;
		submode[x].virt.y = slavesizey;
		submode[x].size.x = GGI_AUTO;
		submode[x].size.y = GGI_AUTO;

		memlen =
		    submode[x].virt.x * submode[x].virt.y *
		    (GT_ByPP(mode.graphtype)) + 64 * 1024;
		/* Add some slack for the -input queues */

		/* Allocate space for the shmem visuals. */
		shmid[x] =
		    shmget(ftok("/dev/null", '0' + x), memlen,
			   IPC_CREAT | 0666);

		/* Open a shared "memory-visual" which is simply a simulated 
		 * display in shared memory.
		 */
		sprintf(text, "display-memory:-input:shmid:%d", shmid[x]);

		if ((memvis[x] = ggiOpen(text, NULL)) == NULL) {
			ggiPanic("Ouch - can't open the shmem target %d !",
				 x);
		}

		err = ggiSetMode(memvis[x], &submode[x]);

		if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
			ggiSetColorfulPalette(memvis[x]);
		}

		/* Check for errors */
		if (err)
			ggiPanic
			    ("Ouch - can't setmode on shmem target %d !",
			     x);

		the_textures[x].vis = memvis[x];
		the_textures[x].filex = submode[x].visible.x;
		the_textures[x].filey = submode[x].visible.y;
		the_textures[x].mapper = PatNomap;
		CheckDB(&the_textures[x]);

		sprintf(text, "display-memory:-input:keyfile:%lu:%d:%s",
			(unsigned long)memlen, x, "/dev/null");
		sprintf(envtext, "GGI_DISPLAY=%s", text);
		putenv(envtext);

		ggiSPrintMode(text, &submode[x]);
		sprintf(envtext, "GGI_DEFMODE=%s", text);
		putenv(envtext);

		if (progarg < argc) {
			printf("face %d execing: %s\n", x, argv[progarg]);
			the_textures[x].pid = spawn_bg(argv[progarg++]);
		} else {
			printf("GGI_DEFMODE=\"%s\"\n", text);
			sprintf(text,
				"display-memory:-input:keyfile:%lu:%d:%s",
				(unsigned long)memlen, x, "/dev/null");
			printf("GGI_DISPLAY=%s\n", text);
		}
	}
	fflush(stderr);
	fflush(stdout);

	{
		/* black, white and red are already defined
		 * at the top of this file
		 */
		ggi_color black_col = { 0x0000, 0x0000, 0x0000 };
		ggi_color white_col = { 0xffff, 0xffff, 0xffff };
		ggi_color red_col = { 0xffff, 0x0000, 0x0000 };
		ggiSetGCForeground(memvis[0],
				   ggiMapColor(memvis[0], &white_col));
		ggiSetGCBackground(memvis[0],
				   ggiMapColor(memvis[0], &black_col));
		ggiPuts(memvis[0], 0, 0, "Keyboard:");
		ggiPuts(memvis[0], 0, 10, "#: Go to Cube control");
		ggiPuts(memvis[0], 0, 20, "Cursor,Home,End: Rotate");
		ggiPuts(memvis[0], 0, 30, "PgUp/PgDown    : Resize");
		ggiPuts(memvis[0], 0, 40, "q: quit s:stop b:backfaces");
		ggiSetGCForeground(memvis[0],
				   ggiMapColor(memvis[0], &red_col));
		ggiDrawHLine(memvis[0], 0, 0, submode[x].visible.x);
		ggiDrawHLine(memvis[0], 0, submode[x].visible.y - 1,
			     submode[x].visible.x);
		ggiDrawVLine(memvis[0], 0, 0, submode[x].visible.y);
		ggiDrawVLine(memvis[0], submode[x].visible.x - 1, 0,
			     submode[x].visible.y);
	}

	ggiFlush(vis);
	noquit = 1;
	is_escape = 0;
	clk = clock();
	stop_speed();
	scale(vissizey / 150.0 / 1.10 * 0.5);	/* Scale to 50% of screen */
	Matrix_times_Matrix((double *) current, (double *) speed);
	stop_speed();
#if 0
	turn_add(0, 2, -M_PI / 180.0);
	turn_add(0, 1, -M_PI / 180.0);
	turn_add(2, 1, -M_PI / 180.0);
#endif

	while (noquit) {

		/* Take care for the blinking border */
		blink = !blink;

		/* Calculate the new turn Matrix */
		Matrix_times_Matrix((double *) current, (double *) speed);

		/* Now transform _all_ points. */
		for (x = 0; x < numpoints; x++) {
			transform_point((double *) current,
					&the_original_points[x],
					&the_points[x]);
		}

		/* Calculate the new normals. */
		for (x = 0; x < the_scene.numpolys; x++) {
			int y, z;

			normvec_poly(the_scene.polys + x);
			midvec_poly(the_scene.polys + x);
			visible_poly(the_scene.polys + x);

			for (y = 0; y < x; y++)
				if (the_scene.polys[x].middle[2] >
				    zorder[y].zmiddle)
					break;
			for (z = x; z > y; z--)
				zorder[z] = zorder[z - 1];
			zorder[y].zmiddle = the_scene.polys[x].middle[2];
			zorder[y].polynum = x;
		}

		/* Check, if textures have changed properties. */
		for (x = 0; x < the_scene.numpolys; x++) {
			ggiGetMode(memvis[x], &submode[x]);

			/* "submode[x].frames != 0" below is an attemp to
			 * kill a race between sub-app and cube3d. When the
			 * sub-app opens the display-memory target it clears
			 * some variables so that the mode returned by
			 * ggiGetMode in cube3d is totally bogus. When the
			 * sub-app later sets a mode, the ggiGetMode call in
			 * cube3d returns sane values again. This "fix"
			 * detects the broken mode returned between ggiOpen
			 * and ggiSetMode in the sub-app. The race is
			 * probably not killed totally, as in ggiGetMode in
			 * cube3d _during_ ggiOpen/ggiSetMode in the sub-app
			 * is not handled. That requires inter-process locks.
			 */

			if (submode[x].frames != 0 &&
			    (the_textures[x].filex !=
			     submode[x].visible.x ||
			     the_textures[x].filey !=
			     submode[x].visible.y)) {
				ggiSetMode(memvis[x], &submode[x]);
				if (GT_SCHEME(submode[x].graphtype) ==
				    GT_PALETTE) {
					ggiSetColorfulPalette(memvis[x]);
				}
				the_textures[x].filex =
				    submode[x].visible.x;
				the_textures[x].filey =
				    submode[x].visible.y;
				if (submode[x].graphtype == mode.graphtype)
					the_textures[x].mapper = PatNomap;
				else
					the_textures[x].mapper = PatRemap;
				CheckDB(&the_textures[x]);
				printf("Mode change on face %d to %dx%d\n",
				       x, the_textures[x].filex,
				       the_textures[x].filey);
			}
		}

		if (autoactive) {
			double best = 0.0;
			for (x = 0; x < the_scene.numpolys; x++) {
				if (the_scene.polys[x].visible > best) {
					best = the_scene.polys[x].visible;
					active_face = x;
				}
			}
		}

		if (backfaces) {
			for (x = 0; x < the_scene.numpolys; x++) {
				int y = zorder[x].polynum;
				if (0.0 > the_scene.polys[y].visible) {
					doblit(&the_scene.polys[y],
					       transparency);
					if (y == active_face) {
						ggiSetGCForeground(vis,
								   blink ?
								   black
								   :
								   (is_escape
								    ? red :
								    white));
						highlight_face(&the_scene.
							       polys[y]);
					}
				}
			}
		} else {
			for (x = 0; x < 6; x++) {
				int y = zorder[x].polynum;
				if (0.0 > the_scene.polys[y].visible
				    && y == active_face) {
					ggiSetGCForeground(vis,
							   blink ? black
							   : (is_escape ?
							      red :
							      white));
					highlight_face(&the_scene.
						       polys[y]);
				}
			}
		}

		for (x = 0; x < 6; x++) {
			int y = zorder[x].polynum;
			if (0.0 <= the_scene.polys[y].visible) {
				doblit(&the_scene.polys[y], transparency);
				if (y == active_face) {
					ggiSetGCForeground(vis,
							   blink ? black
							   : (is_escape ?
							      red :
							      white));
					highlight_face(&the_scene.
						       polys[y]);
				}
			}
		}

		if (doalign)
			if (align(&the_scene.polys[active_face]))
				doalign = 0;

		tv.tv_sec = tv.tv_usec = 0;

		while (ggiEventPoll(vis, emAll, &tv)) {
			ggi_event event;
			ggiEventRead(vis, &event, emAll);

			if (event.any.type == evKeyPress) {
				if (!is_escape) {
					if (event.key.sym == '#')
						is_escape = 1;
					else {
					      sendit:
						giiEventSend(ggiJoinInputs
							     (memvis
							      [active_face],
							      NULL),
							     &event);
						is_escape = 0;
						continue;
					}
				} else {
					switch (event.key.sym) {
					case GIIK_Left:
						turn_add(0, 2,
							 -M_PI / 180.0);
						break;
					case GIIK_Right:
						turn_add(0, 2,
							 M_PI / 180.0);
						break;
					case GIIK_Up:
						turn_add(1, 2,
							 -M_PI / 180.0);
						break;
					case GIIK_Down:
						turn_add(1, 2,
							 M_PI / 180.0);
						break;
					case GIIK_Home:
						turn_add(0, 1,
							 -M_PI / 180.0);
						break;
					case GIIK_End:
						turn_add(0, 1,
							 M_PI / 180.0);
						break;
					case GIIK_PageUp:
						scale(1.01);
						break;
					case GIIK_PageDown:
						scale(1.0 / 1.01);
						break;
					case 's':
					case 'S':
						stop_speed();
						break;
					case 'c':
					case 'C':
						doalign = !doalign;
						break;
					case 'b':
					case 'B':
						backfaces = !backfaces;
						break;
					case 'a':
					case 'A':
						autoactive = !autoactive;
						break;
					case 't':
					case 'T':
						transparency =
						    !transparency;
						break;
					case '0':
						autoactive = 0;
						active_face = 0;
						break;
					case '1':
						autoactive = 0;
						active_face = 1;
						break;
					case '2':
						autoactive = 0;
						active_face = 2;
						break;
					case '3':
						autoactive = 0;
						active_face = 3;
						break;
					case '4':
						autoactive = 0;
						active_face = 4;
						break;
					case '5':
						autoactive = 0;
						active_face = 5;
						break;
					case 'q':
					case 'Q':
						noquit = 0;
						break;
					case '#':
						goto sendit;
					case 'f':
					case 'F':
						printf("FPS:%f\n",
						       (double) frcnt /
						       (clock() - clk +
							1.0) *
						       CLOCKS_PER_SEC);
						clk = clock();
						frcnt = 0;
						break;
					default:
						printf
						    ("Unknown command key %d\n",
						     event.key.sym);
					case GIIK_Enter:
						is_escape = 0;
						break;
					}
				}
			} else {
				giiEventSend(ggiJoinInputs
					     (memvis[active_face], NULL),
					     &event);
			}
		}
		if (frame >= 0)
			ggiSetDisplayFrame(vis, frame);
		ggiFlush(vis);
		frcnt++;
		if (frame >= 0)
			ggiSetWriteFrame(vis, frame = !frame);
		ggiSetGCForeground(vis, black);
		ggiFillscreen(vis);
	}

	for (x = 0; x < 6; x++) {
#ifdef HAVE_KILL
		if (the_textures[x].pid > 0)
			kill(the_textures[x].pid, SIGHUP);
#elif defined(HAVE_WINDOWS_H)
		if (the_textures[x].pid != 0)
			TerminateProcess((HANDLE)the_textures[x].pid, 1);
#endif
		ggiClose(memvis[x]);

		/* this makes the memory detach when all programs using this memory
		 * complete (according to SVR4 standard).
		 * note - this may not work on all platforms....
		 * note2: This may _not be called at an earlier place.
		 * Calling before our own shmat() will destroy _instantly_,
		 * calling before the other programs are running will deny
		 * the shmem to them.
		 */
		shmctl(shmid[x], IPC_RMID, NULL);
	}
	ggiClose(vis);
	ggiExit();
	return 0;
}
