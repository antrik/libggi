/*
 * h3600 GGI calibration Application
 * Author: Tobias Hunger
 * based on xcalibrate from Charles Flynn
 */

#define SCANCHAR 1
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/ioctl.h>

#include <assert.h>

#include <ggi/ggi.h>

typedef struct {
	ushort x;
	ushort y;
} TS_POINT;

#define TRANSFORMATION_UNITS_PER_PIXEL 4

typedef struct {
	/*
	 * Coefficients for the transformation formulas:
	 *
	 *     m = (ax + by + c) / s
	 *     n = (dx + ey + f) / s
	 *
	 * These formulas will transform a device point (x, y) to a
	 * screen point (m, n) in fractional pixels.  The fraction
	 * is 1 / TRANSFORMATION_UNITS_PER_PIXEL.
	 */

	int a, b, c, d, e, f, s;
} TRANSFORMATION_COEFFICIENTS;

typedef struct {
	TS_POINT screen, device;
} CALIBRATION_PAIR;

#define NIL (0)

int rvFlag = 0;

/* Note: ITSY geometry screenW=320, screenH=200; H3600 Width=320 Height=240*/
#define SZ_XHAIR                50	/* Num of pixels across xhair */

static ggi_visual_t vis;
static ggi_mode vis_mode;

ggi_pixel white;
ggi_pixel black;

static int CalcTransformationCoefficientsBest(CALIBRATION_PAIR * cp,
				       TRANSFORMATION_COEFFICIENTS * ptc,
				       int points)
{
#if 0
	ptc->s = 1 << 16;

	ptc->a = (ptc->s * (cp[2].screen.x - cp[0].screen.x)) /
	    (cp[2].device.x - cp[0].device.x);
	ptc->b = 0;
	ptc->c = ptc->s * cp[2].screen.x - ptc->a * cp[2].device.x;

	ptc->d = 0;
	ptc->e = (ptc->s * (cp[2].screen.y - cp[0].screen.y)) /
	    (cp[2].device.y - cp[0].device.y);
	ptc->f = ptc->s * cp[2].screen.y - ptc->e * cp[2].device.y;

	return 0;
#else
	/*
	 * Mike Klar <> came up with a best-fit solution that works best.
	 */

	int i;

	double Sx = 0, Sy = 0, Sxy = 0, Sx2 = 0, Sy2 = 0, Sm = 0, Sn =
	    0, Smx = 0, Smy = 0, Snx = 0, Sny = 0, S = 0;
	double t1, t2, t3, t4, t5, t6, q;

	/*
	 * Do a best-fit calculation for as many points as we want, as
	 * opposed to an exact fit, which can only be done against 3 points.
	 *
	 * The following calculates various sumnations of the sample data
	 * coordinates.  For purposes of naming convention, x and y
	 * refer to device coordinates, m and n refer to screen
	 * coordinates, S means sumnation.  x2 and y2 are x squared and
	 * y squared, S by itself is just a count of points (= sumnation
	 * of 1).
	 */

	for (i = 0; i < points; i++) {
		Sx += cp[i].device.x;
		Sy += cp[i].device.y;
		Sxy += cp[i].device.x * cp[i].device.y;
		Sx2 += cp[i].device.x * cp[i].device.x;
		Sy2 += cp[i].device.y * cp[i].device.y;
		Sm += cp[i].screen.x;
		Sn += cp[i].screen.y;
		Smx += cp[i].screen.x * cp[i].device.x;
		Smy += cp[i].screen.x * cp[i].device.y;
		Snx += cp[i].screen.y * cp[i].device.x;
		Sny += cp[i].screen.y * cp[i].device.y;
		S += 1;
	}

	/*
	 * Next we solve the simultaneous equations (these equations minimize
	 * the sum of the square of the m and n error):
	 *
	 *    | Sx2 Sxy Sx |   | a d |   | Smx Snx |
	 *    | Sxy Sy2 Sy | * | b e | = | Smy Sny |
	 *    | Sx  Sy  S  |   | c f |   | Sm  Sn  |
	 *
	 * We could do the matrix solution in code, but that leads to several
	 * divide by 0 conditions for cases where the data is truly solvable
	 * (becuase those terms cancel out of the final solution), so we just
	 * give the final solution instread.  t1 through t6 and q are just
	 * convenience variables for terms that are used repeatedly - we could
	 * calculate each of the coefficients directly at this point with a
	 * nasty long equation, but that would be extremly inefficient.
	 */

	t1 = Sxy * Sy - Sx * Sy2;
	t2 = Sxy * Sx - Sx2 * Sy;
	t3 = Sx2 * Sy2 - Sxy * Sxy;
	t4 = Sy2 * S - Sy * Sy;
	t5 = Sx * Sy - Sxy * S;
	t6 = Sx2 * S - Sx * Sx;

	q = t1 * Sx + t2 * Sy + t3 * S;

	/*
	 * If q = 0, then the data is unsolvable.  This should only happen
	 * when there are not enough unique data points (less than 3 points
	 * will give infinite solutions), or at least one of the 
	 * coefficients is infinite (which would indicate that the same
	 * device point represents an infinite area of the screen, probably
	 * as a result of the same device data point given for 2 different
	 * screen points).  The first condition should never happen, since
	 * we're always feeding in at least 3 unique screen points.  The
	 * second condition would probably indicate bad user input or the
	 * touchpanel device returning bad data.
	 */

	if (q == 0)
		return -1;

	ptc->s = 1 << 16;
	ptc->a =
	    ((t4 * Smx + t5 * Smy + t1 * Sm) / q + 0.5 / 65536) * ptc->s;
	ptc->b =
	    ((t5 * Smx + t6 * Smy + t2 * Sm) / q + 0.5 / 65536) * ptc->s;
	ptc->c =
	    ((t1 * Smx + t2 * Smy + t3 * Sm) / q + 0.5 / 65536) * ptc->s;
	ptc->d =
	    ((t4 * Snx + t5 * Sny + t1 * Sn) / q + 0.5 / 65536) * ptc->s;
	ptc->e =
	    ((t5 * Snx + t6 * Sny + t2 * Sn) / q + 0.5 / 65536) * ptc->s;
	ptc->f =
	    ((t1 * Snx + t2 * Sny + t3 * Sn) / q + 0.5 / 65536) * ptc->s;

	/*
	 * Finally, we check for overflow on the fp to integer conversion,
	 * which would also probably indicate bad data.
	 */

	if (((unsigned)ptc->a == 0x80000000) ||
	    ((unsigned)ptc->b == 0x80000000) ||
	    ((unsigned)ptc->c == 0x80000000) ||
	    ((unsigned)ptc->d == 0x80000000) ||
	    ((unsigned)ptc->e == 0x80000000) ||
	    ((unsigned)ptc->f == 0x80000000))
	{
		return -1;
	}

	return 0;
#endif
}



static void drawPlus(int xcoord, int ycoord, int delete)
{
	int hx_coord1, hy_coord1, hx_coord2, hy_coord2;
	int vx_coord1, vy_coord1, vx_coord2, vy_coord2;

	/* clear screen */
	ggiSetGCForeground(vis, black);
	ggiFillscreen(vis);

	/*      if (rotated) {
	   int temp = xcoord;
	   xcoord = ycoord;
	   ycoord = SCREEN_WIDTH - temp;
	   }
	 */
	/* work out the horizontal and vertical line coords */

	/* horizontal line */
	hx_coord1 = xcoord - (SZ_XHAIR / 2);
	hx_coord2 = xcoord + (SZ_XHAIR / 2);
	hy_coord1 = hy_coord2 = ycoord;
	/* vertical line */
	vx_coord1 = vx_coord2 = xcoord;
	vy_coord1 = ycoord - (SZ_XHAIR / 2);
	vy_coord2 = ycoord + (SZ_XHAIR / 2);


	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);

	/* draw horizontal line */
	ggiDrawHLine(vis, hx_coord1, hy_coord1, SZ_XHAIR);

	/* draw vertical line */
	ggiDrawVLine(vis, vx_coord1, vy_coord1, SZ_XHAIR);
}

/* Check that the response is in the crosshairs */

static int nearPlus(int xcoord, int ycoord, ggi_event point)
{
	/*
	   int hx_coord1,hy_coord1,hx_coord2,hy_coord2;
	   int vx_coord1,vy_coord1,vx_coord2,vy_coord2;
	   int x, y;

	   #if 0
	   printf("Old calibration:\n");
	   printf("  xscale:%5d, xtrans:%5d\n", raw_cal.xscale,
	   raw_cal.xtrans);
	   printf("  yscale:%5d, xtrans:%5d\n", raw_cal.yscale,
	   raw_cal.ytrans);
	   printf("xyswap:%s\n", (raw_cal.xyswap == 0 ? "N" : "Y"));
	   #endif

	   hx_coord1= xcoord - (SZ_XHAIR/2);
	   hx_coord2= xcoord + (SZ_XHAIR/2);

	   vy_coord1= ycoord - (SZ_XHAIR/2);
	   vy_coord2= ycoord + (SZ_XHAIR/2);

	   x = ( ( raw_cal.xscale * point.val.value[0] ) >> 8 ) + raw_cal.xtrans;
	   y = ( ( raw_cal.yscale * point.val.value[1] ) >> 8 ) + raw_cal.ytrans;
	   #if 0
	   printf("Should be (%d,%d): point.x=%d point.y=%d\t actually is (%d,%d)\n", 
	   xcoord, ycoord, point.x, point.y, x, y);
	   printf("X: %d - %d - %d      Y: %d - %d - %d\n",
	   hx_coord1, x, hx_coord2, vy_coord1, y, vy_coord2);
	   #endif
	   return ((hx_coord1 < x) && (x < hx_coord2) &&
	   (vy_coord1 < y) && (y < vy_coord2));
	 */

	return 1;
}

static int do_calibration(void)
{
	int k, xsum, ysum, cnt;
	ggi_event event;
	ggi_event_mask mask = emValuator | emPtrButton;
	TS_POINT screenCalPos[5];
#define MAX_CAL_POINTS	5
	int xa[MAX_CAL_POINTS], ya[MAX_CAL_POINTS];

	/* Screen Offsets  for test calibration */
#define OFF1 50
#define OFF2 30

	screenCalPos[0].x = OFF1;
	screenCalPos[0].y = OFF2;

	screenCalPos[1].x = vis_mode.visible.x - OFF1 - 1;
	screenCalPos[1].y = OFF2;

	screenCalPos[2].x = vis_mode.visible.x - OFF1 - 1;
	screenCalPos[2].y = vis_mode.visible.y - OFF2 - 1;

	screenCalPos[3].x = OFF1;
	screenCalPos[3].y = vis_mode.visible.y - OFF2 - 1;

	screenCalPos[4].x = vis_mode.visible.x / 2;
	screenCalPos[4].y = vis_mode.visible.y / 2;

	/*
	 *   Enter here with ts events pertaining to screen coords screenCalPos[k]
	 */

	for (k = 0; k < MAX_CAL_POINTS; k++) {
		int keeprunning = 1;
		/*        
		 * for each screen position take the average of 5 points.
		 */

		fflush(stdout);
		drawPlus(screenCalPos[k].x, screenCalPos[k].y, 0);

		cnt = xsum = ysum = 0;
		xa[k] = ya[k] = 0;

		/* now loop until we have 5 samples - */
		while (keeprunning) {
			ggiEventRead(vis, &event, mask);

			switch (event.any.type) {
			case evPtrButtonRelease:
				if (cnt > 5)
					keeprunning = 0;
				break;
			default:
				if (!nearPlus
				    (screenCalPos[k].x, screenCalPos[k].y,
				     event))
					continue;

				/* accumulate the x coords */
				xsum += event.val.value[0];
				ysum += event.val.value[1];
				/* increment the event counter */
				++cnt;
				break;
			}
			fflush(stdout);
		}		/* endwhile */

		/* delete the plus */
		drawPlus(screenCalPos[k].x, screenCalPos[k].y, 1);

		/*
		 *  Enter here with - agregate of the x,y coords stored in xsum & ysum.
		 */


		/* take the average of the x and y points */
		xa[k] = xsum / cnt;
		ya[k] = ysum / cnt;
	}

	/* Enter here with the average x,y coords of the MAX_CAL_POINTS */
	{
		CALIBRATION_PAIR cp[MAX_CAL_POINTS];
		TRANSFORMATION_COEFFICIENTS tc;

		cp[0].screen.x = screenCalPos[0].x;
		cp[0].screen.y = screenCalPos[0].y;
		cp[0].device.x = ya[0];
		cp[0].device.y = xa[0];

		cp[1].screen.x = screenCalPos[1].x;
		cp[1].screen.y = screenCalPos[1].y;
		cp[1].device.x = ya[1];
		cp[1].device.y = xa[1];

		cp[2].screen.x = screenCalPos[2].x;
		cp[2].screen.y = screenCalPos[2].y;
		cp[2].device.x = ya[2];
		cp[2].device.y = xa[2];

		cp[3].screen.x = screenCalPos[3].x;
		cp[3].screen.y = screenCalPos[3].y;
		cp[3].device.x = ya[3];
		cp[3].device.y = xa[3];

		cp[4].screen.x = screenCalPos[4].x;
		cp[4].screen.y = screenCalPos[4].y;
		cp[4].device.x = ya[4];
		cp[4].device.y = xa[4];

		CalcTransformationCoefficientsBest(cp, &tc,
						   MAX_CAL_POINTS);

		printf("Calibration: %d %d %d %d %d %d %d\n",
		       tc.a, tc.b, tc.c, tc.d, tc.e, tc.f, tc.s);

		for (k = 0; k < MAX_CAL_POINTS; k++)
			printf
			    ("Calibrationdata[%d]: (%d,%d) => (%d,%d): (%d, %d)\n",
			     k, cp[k].device.x, cp[k].device.y,
			     cp[k].screen.x, cp[k].screen.y,
			     (cp[k].device.x * tc.a +
			      cp[k].device.y * tc.b + tc.c) / tc.s,
			     (cp[k].device.x * tc.b +
			      cp[k].device.y * tc.e + tc.f) / tc.s);
	}

	return 0;
}

static int initGGI(void)
{
	ggi_color map;

	ggiParseMode("", &vis_mode);

	if (ggiInit() != 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	vis = ggiOpen(NULL, NULL);

	if (vis == NULL) {
		ggiPanic("Failed to open visual.\n");
	}

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	ggiCheckMode(vis, &vis_mode);

	if (ggiSetMode(vis, &vis_mode) < 0) {
		ggiPanic("mode refused.\n");
	}

	/* Find the colors "white" and "black". */
	map.r = map.g = map.b = 0xFFFF;

	white = ggiMapColor(vis, &map);

	map.r = map.g = map.b = 0x0;
	black = ggiMapColor(vis, &map);

	return 1;
}

int main(int argc, char **argv)
{
	if (initGGI() < 0) {
		printf("Unable to GGIinit\n");
		exit(1);
	}

	do_calibration();

	/* close GGI */
	ggiClose(vis);
	ggiExit();

	/* everything OK */
	return 1;
}
