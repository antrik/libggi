/* $Id: modelist.inc,v 1.5 2005/01/31 18:34:42 cegger Exp $
******************************************************************************

   Functions to handle GGI_AUTO for targets with a list of modes.

   Copyright (C) 1998 Marcus Sundberg - marcus@ggi-project.org

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#ifdef WANT_MODELIST
#define WANT_CHECKONEBPP
#define WANT_GETHIGHBPP
#define WANT_CHECKAUTOBPP
#endif

#ifdef WANT_CHECKONEBPP
static int _GGIcheckonebpp(ggi_visual * vis, ggi_mode * tm,
			   ggi_modelistmode * gm)
{
	int i = 0, bestw = 0, besth = 0;
	int wantw = tm->visible.x, wanth = tm->visible.y;
	for (i = 0; gm[i].bpp != 0; i++) {
		DPRINT_MODE
		    ("_GGIcheckonebpp, checking: %dx%d, wanting: %dx%d, gm[i].gt=%d, tm->graphtype=%d, bestw=%d, besth=%d\n",
		     gm[i].x, gm[i].y, wantw, wanth, gm[i].gt,
		     tm->graphtype, bestw, besth);
		if (gm[i].gt != tm->graphtype) {
			DPRINT_MODE
			    ("_GGIcheckonebpp, wanted: 0x%x, skipping 0x%x\n",
			     tm->graphtype, gm[i].gt);
			continue;
		}
		if ((gm[i].x == wantw || wantw == GGI_AUTO) &&
		    (gm[i].y == wanth || wanth == GGI_AUTO)) {
			tm->visible.x = gm[i].x;
			tm->visible.y = gm[i].y;
			return 0;
		}
		if (((gm[i].x >= bestw && bestw <= wantw)
		     || wantw == GGI_AUTO
		     || (bestw >= wantw && gm[i].x >= wantw)) &&
		    ((gm[i].y >= besth && besth <= wanth)
		     || wanth == GGI_AUTO
		     || (besth >= wanth && gm[i].y >= wanth))) {
			DPRINT_MODE("_GGIcheckonebpp, best: %dx%d\n",
				    gm[i].x, gm[i].y);
			bestw = gm[i].x;
			besth = gm[i].y;
		}
	}
	if (bestw == 0)
		return 1;
	tm->visible.x = bestw;
	tm->visible.y = besth;
	return GGI_ENOMATCH;
}
#endif

#ifdef WANT_GETHIGHBPP
static int _GGIgethighbpp(ggi_visual * vis, ggi_mode * tm,
			  ggi_modelistmode * gm)
{
	int i = 0, bestw = 0, besth = 0, bestbpp = 0, bestgt = GT_AUTO;
	int wantw = tm->visible.x, wanth = tm->visible.y,
	    wantbpp = GT_DEPTH(tm->graphtype);
	for (i = 0; gm[i].bpp != 0; i++) {
		if (((gm[i].x >= bestw &&
		      bestw <= wantw) || wantw == GGI_AUTO) &&
		    ((gm[i].y >= besth &&
		      besth <= wanth) || wanth == GGI_AUTO) &&
		    (gm[i].bpp >= bestbpp && bestbpp <= wantbpp)) {
			bestw = gm[i].x;
			besth = gm[i].y;
			bestbpp = gm[i].bpp;
			bestgt = gm[i].gt;
			continue;
		}
	}
	tm->visible.x = bestw;
	tm->visible.y = besth;
	tm->graphtype = bestgt;

	return 0;

}
#endif

#ifdef WANT_CHECKAUTOBPP
static int _GGIcheckautobpp(ggi_visual * vis, ggi_mode * tm,
			    ggi_modelistmode * gm)
{
	int i = 0, ret = 0, bestw = 0, besth = 0, bestbpp = 0, bestgt =
	    GT_AUTO, gotmode = 0;
	int wantw = tm->visible.x, wanth = tm->visible.y;
	for (i = 0; gm[i].bpp != 0; i++) {
		if ((gm[i].x == wantw || wantw == GGI_AUTO) &&
		    (gm[i].y == wanth || wanth == GGI_AUTO)) {
			if (gm[i].bpp >= bestbpp) {
				bestw = gm[i].x;
				besth = gm[i].y;
				bestbpp = gm[i].bpp;
				bestgt = gm[i].gt;
				gotmode = 1;
			}
		}
		if (gotmode)
			continue;
		if (((gm[i].x >= bestw &&
		      bestw <= wantw) || wantw == GGI_AUTO) &&
		    ((gm[i].y >= besth &&
		      besth <= wanth) || wanth == GGI_AUTO) &&
		    gm[i].bpp >= bestbpp) {
			bestw = gm[i].x;
			besth = gm[i].y;
			bestbpp = gm[i].bpp;
			bestgt = gm[i].gt;
		}
	}
	if (gotmode) {
		ret = 0;
	} else {
		ret = -1;
	}
	tm->visible.x = bestw;
	tm->visible.y = besth;
	tm->graphtype = bestgt;

	return ret;
}
#endif
