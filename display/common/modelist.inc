/* $Id: modelist.inc,v 1.18 2006/10/25 09:32:06 orzo Exp $
******************************************************************************

   Functions to handle GGI_AUTO for targets with a list of modes.

   Copyright (C) 1998 Marcus Sundberg		[marcus@ggi-project.org]
   Copyright (C) 2005 Joseph Crayne		[oh.hello.joe@gmail.com]

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
static int _GGIcheckonebpp(struct ggi_visual * vis, ggi_mode * tm,
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
static int _GGIgethighbpp(struct ggi_visual * vis, ggi_mode * tm,
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
static int _GGIcheckautobpp(struct ggi_visual * vis, ggi_mode * tm,
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

#ifdef WANT_MODELIST2

static
void _GGI_modelist_default_adjust(ggi_mode_padded * req,
				  ggi_mode_padded * recognizer,
				  void *param)
{
}

static
ggi_modelist *_GGI_modelist_create(int n)
{
	ggi_modelist *retv;
	retv = malloc(sizeof(ggi_modelist));
	retv->list = malloc(n * sizeof(ggi_mode_padded));
	retv->n = 0;
	retv->max_n = n;
	retv->adjust = _GGI_modelist_default_adjust;
	retv->adjust_param = NULL;
	return retv;
}

static
void _GGI_modelist_destroy(ggi_modelist * ml)
{
	free(ml->list);
	free(ml);
}

static
void _GGI_modelist_append(ggi_modelist * ml, ggi_mode_padded * m)
{
	if (ml->n == ml->max_n) {
		ml->max_n++;
		ml->list =
		    realloc(ml->list, ml->max_n * sizeof(ggi_mode_padded));
	}

	memcpy(&ml->list[ml->n], m, sizeof(ggi_mode_padded));
	ml->n++;

}
#endif


#if defined WANT_GENERIC_CHECKMODE || defined WANT_LIST_CHECKMODE

/* In the following functions, lower scores are better
 * than higher ones... */

/* This function is for adding the less important
 * aspect of a score that is usually the distance
 * from the match. */
static void _GGI_write_subscore(int *score, int subscore)
{
	/* I assume that score as given is a value between 0 and 3 */

	/* Note that I'm wasting a bit due to the fact
	 * that the scores are signed... */
	/* shift to the high 3 bits */
	int shift = 8 * sizeof(int) - 3;
	*score <<= shift;
	/* let the rest of the bits be determined by the distance
	 * from match */
	*score += subscore;
}

/* This function currently considers only the color depth
 * of the graphtype. */
static int _GGI_score_gt_depth(ggi_graphtype req, ggi_graphtype a)
{
	int score;
	if (GT_DEPTH(a) == GT_DEPTH(req))
		score = 0;
	else if (GT_DEPTH(a) > GT_DEPTH(req))
		score = 1;
	else
		score = 2;

	_GGI_write_subscore(&score,
			    abs(GT_DEPTH(a) - GT_DEPTH(req)));

	return score;
}

/* This function currently considers only the color scheme
 * of the graphtype. */
static int _GGI_score_gt_scheme(ggi_graphtype req, ggi_graphtype a)
{
	int score;
	if (GT_SCHEME(a) == GT_SCHEME(req)) {
		score = 0;
	} else {
		switch (GT_SCHEME(req)) {
		case GT_TEXT:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_GREYSCALE:
			case GT_PALETTE:
			case GT_STATIC_PALETTE:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;

			default:
				score = 2;
				break;
			}

		case GT_TRUECOLOR:
			switch (GT_SCHEME(a)) {
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_GREYSCALE:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_PALETTE:
			case GT_STATIC_PALETTE:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_PALETTE:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_STATIC_PALETTE:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_PALETTE:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_SUBSAMPLE_YUV:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_SUBSAMPLE_U_YCRBR:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_SUBSAMPLE_U_YCRBR:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_S_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		case GT_SUBSAMPLE_S_YCRBR:
			switch (GT_SCHEME(a)) {
			case GT_TRUECOLOR:
			case GT_SUBSAMPLE_YUV:
			case GT_SUBSAMPLE_U_YCRBR:
				score = 1;
				break;
			default:
				score = 2;
				break;
			}

		default:
			score = 2;
			break;
		}
	}


	_GGI_write_subscore(&score,
			    abs(GT_SCHEME(a) - GT_SCHEME(req)));

	return score;
}

static int _GGI_score_resolution(ggi_coord req, ggi_coord a)
{
	int score;
	if (req.x == a.x && req.y == a.y)
		score = 0;
	else if (a.x >= req.x && a.y >= req.y)
		score = 1;
	else
		score = 2;

	_GGI_write_subscore(&score,
			    abs(req.x - a.x) + abs(req.y - a.y));

	return score;
}

static int _GGI_score_frames(int32_t req, int32_t a)
{
	int score;
	if (a == req)
		score = 0;
	else if (a > req)
		score = 1;
	else
		score = 2;

	_GGI_write_subscore(&score, abs(a - req));

	return score;
}

/* This function currently makes no account of fontsize for text modes.
 * It works by using _GGI_score_* helper functions which assign lower 
 * scores to more desirable things. 
 * Returns: <0, ==0, >0 if a<b, a==b, or a>b respectively. */
static int _GGI_default_mode_cmp(ggi_mode * requested, ggi_mode * a,
				 ggi_mode * b)
{
	/* Perhaps there should be a user mode-compare hook called here 
	 * or maybe let the user override the default_mode_cmp function
	 * entirely and chain to it only if they want. */

  int i;
  int score[2] = {0,0};

  /* score[0] = score based on user-specified fields
   * score[1] = score based on unspecified fields
   */

  i = requested->graphtype==GT_AUTO;
	score[i] = _GGI_score_gt_depth(requested->graphtype, a->graphtype)
             - _GGI_score_gt_depth(requested->graphtype, b->graphtype);
  
  i = requested->graphtype==GGI_AUTO;
  if(!score[i]) 
    score[i] = _GGI_score_gt_scheme(requested->graphtype, a->graphtype)
                   - _GGI_score_gt_scheme(requested->graphtype, b->graphtype);

  i = requested->visible==GGI_AUTO;
  if(!score[i])
    score[i] = _GGI_score_resolution(requested->visible, a->visible)
                   - _GGI_score_resolution(requested->visible, b->visible); 

  i = requested->virt==GGI_AUTO;
  if(!score[i])
    score[i] = _GGI_score_resolution(requested->virt, a->virt)
                   - _GGI_score_resolution(requested->virt, b->virt);

  i = requested->frames==GGI_AUTO;
  if(!score[i])
    score[i] = _GGI_score_frames(requested->frames, a->frames)
               - _GGI_score_frames(requested->frames, b->frames);

	/* Prefer larger physical sizes to smaller ones */
  if(!score[1])
	  score[1] = b->size.x * b->size.y  -  a->size.x * a->size.y;

	return score[0] ? score[0] : score[1];
}

static int _ret0( ggi_mode *mode, intptr_t mode_extra1, intptr_t mode_extra2, 
		  void *param)
{
	return 0;
}

static ggi_checkmode_t *_GGI_generic_checkmode_create(void)
{
	ggi_checkmode_t * cm;
	cm = malloc( sizeof( ggi_checkmode_t ) );
	cm->user_cmp = _ret0 ;
	cm->user_param = NULL;
	return cm;
}

static
void _GGI_generic_checkmode_destroy( ggi_checkmode_t * cm)
{
	free( cm );
}

static
void _GGI_generic_checkmode_init(ggi_checkmode_t *cm, ggi_mode *tm)
{
	cm->sug.graphtype = GT_INVALID; /* no suggested mode yet */
	/* copy requested mode */
	memcpy( &cm->req, tm, sizeof(ggi_mode) );
	cm->mode_extra = 0;
}

static 
void _GGI_generic_checkmode_update(ggi_checkmode_t * cm, 
				   ggi_mode * p,
				   intptr_t mode_extra)
{
	int ret;
	int maxamize_x, maxamize_y, maxamize_depth;

	/* make the default to maximize the resolution... */
	maxamize_x = (cm->req.visible.x == GGI_AUTO);
	maxamize_y = (cm->req.visible.y == GGI_AUTO);
	maxamize_depth = ( cm->req.graphtype == GT_AUTO);

	/* First time... nothing to compare. */
	if( cm->sug.graphtype == GT_INVALID )  {
		/* store suggested mode */
		memcpy( &cm->sug, p, sizeof(ggi_mode) );
		cm->mode_extra = mode_extra;
		return;
	}

	if( maxamize_x ) {
		if (p->visible.x > cm->sug.visible.x)
			cm->req.visible.x = p->visible.x;
		else
			cm->req.visible.x = cm->sug.visible.x;
	}
	if (maxamize_y ) {
		if (p->visible.y > cm->sug.visible.y)
			cm->req.visible.y = p->visible.y;
		else
			cm->req.visible.y = cm->sug.visible.y;
	}
	if( maxamize_depth ) {
		if( GT_DEPTH(p->graphtype) > GT_DEPTH(cm->sug.graphtype) )
			cm->req.graphtype = p->graphtype;
		else
			cm->req.graphtype = cm->sug.graphtype;
	}

	ret =_GGI_default_mode_cmp(&cm->req, p, &cm->sug);

	/* If the modes are indistinguishable, let the user
	 * decide with the user_cmp hook... */
	if( ret == 0 ) 
		ret = cm->user_cmp( &cm->req, 
				    mode_extra, 
				    cm->mode_extra,
				    cm->user_param );
	if( ret < 0 ) {
		memcpy( &cm->sug, p, sizeof(ggi_mode) );
		cm->mode_extra = mode_extra;
	}

	/* restore to GGI_AUTO after maximizing */
	if (maxamize_x)
		cm->req.visible.x = GGI_AUTO;
	if (maxamize_y)
		cm->req.visible.y = GGI_AUTO;
	if( maxamize_depth ) 
		cm->req.graphtype = GT_AUTO;
}

static
int _GGI_generic_checkmode_finish(ggi_checkmode_t * cm,
				  ggi_mode * mode,
				  intptr_t * mode_extra )
{
	int retv;

	/* We use GT_INVALID as a marker to mean that checkmode_update()
	 * was never called and there are presumably no modes available. */
	if( cm->sug.graphtype == GT_INVALID )
		retv = GGI_ENODEVICE;
	
	/* Did we find a perfect match?  */
	else if ((cm->req.frames == GGI_AUTO ||
	     cm->sug.frames == cm->req.frames)
	    && (cm->req.visible.x == GGI_AUTO ||
		cm->sug.visible.x == cm->req.visible.x)
	    && (cm->req.visible.y == GGI_AUTO ||
		cm->sug.visible.y == cm->req.visible.y)
	    && (cm->req.virt.x == GGI_AUTO ||
		cm->sug.virt.x == cm->req.virt.x)
	    && (cm->req.virt.y == GGI_AUTO ||
		cm->sug.virt.y == cm->req.virt.y)
	    && (cm->req.graphtype == GT_AUTO ||
		cm->sug.graphtype == cm->req.graphtype)
	    && (cm->req.dpp.x == GGI_AUTO ||
		cm->sug.dpp.x == cm->req.dpp.x)
	    && (cm->req.dpp.y == GGI_AUTO ||
		cm->sug.dpp.y == cm->req.dpp.y)
	    && (cm->req.size.x == GGI_AUTO ||
		cm->sug.size.x == cm->req.size.x)
	    && (cm->req.size.y == GGI_AUTO ||
		cm->sug.size.y == cm->req.size.y))

		retv = GGI_OK;
	else
		retv = GGI_ENOMATCH;

	memcpy(mode, &cm->sug, sizeof(ggi_mode));
	if( mode_extra != NULL )
		*mode_extra = cm->mode_extra;
	return retv;
}
#endif

#ifdef WANT_LIST_CHECKMODE

/* This implements checkmode simply by iterating over the
 * list and finding the member that is considered "least"
 * by the ordering imposed by _GGI_default_mode_cmp(). */
static
int _GGI_modelist_checkmode(ggi_modelist * ml, ggi_mode_padded * tm)
{
	ggi_mode_padded *p = ml->list;
	int i;
	int retv;

	ggi_checkmode_t *cm =
		_GGI_generic_checkmode_create();

	_GGI_generic_checkmode_init( cm, &tm->mode );

	for (i = 0; i < ml->n; i++, p++) {

		/* adjust p for wildcard behavior */
		ml->adjust(tm, p, ml->adjust_param);

		_GGI_generic_checkmode_update( cm, &p->mode, 
					(intptr_t)p->user_data );
	}

	retv = _GGI_generic_checkmode_finish( cm, &tm->mode, 
					(intptr_t *)&tm->user_data );
	_GGI_generic_checkmode_destroy(cm);
	return retv;
}
#endif

// vim: ft=c
