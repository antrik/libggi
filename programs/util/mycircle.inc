/*
 * Kept around for demo purposes.
 */
int myDrawCircle(ggi_visual_t vis, int xcenter, int ycenter, int radius)
{
	int x, y, od, md, sd;

	/* Should we clip more exactly to the 4 quadrants ? */

	x = radius;
	y = md = 0;
	while (x >= y) {
		ggiDrawPixel(vis, xcenter - y, ycenter - x);
		ggiDrawPixel(vis, xcenter + y, ycenter - x);
		od = md + y + y + 1;
		sd = od - x - x - 1;
		y++;
		md = od;
		if (abs(sd) < abs(od)) {
			x--;
			md = sd;
		}
	}
	x = radius;
	y = md = 0;
	while (x >= y) {
		ggiDrawPixel(vis, xcenter + x, ycenter - y);
		ggiDrawPixel(vis, xcenter - x, ycenter - y);
		od = md + y + y + 1;
		sd = od - x - x - 1;
		y++;
		md = od;
		if (abs(sd) < abs(od)) {
			x--;
			md = sd;
		}
	}
	x = radius;
	y = md = 0;
	while (x >= y) {
		ggiDrawPixel(vis, xcenter - x, ycenter + y);
		ggiDrawPixel(vis, xcenter + x, ycenter + y);
		od = md + y + y + 1;
		sd = od - x - x - 1;
		y++;
		md = od;
		if (abs(sd) < abs(od)) {
			x--;
			md = sd;
		}
	}
	x = radius;
	y = md = 0;
	while (x >= y) {
		ggiDrawPixel(vis, xcenter + y, ycenter + x);
		ggiDrawPixel(vis, xcenter - y, ycenter + x);
		od = md + y + y + 1;
		sd = od - x - x - 1;
		y++;
		md = od;
		if (abs(sd) < abs(od)) {
			x--;
			md = sd;
		}
	}

	return (0);
}
