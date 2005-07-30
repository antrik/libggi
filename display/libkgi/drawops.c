/* $Id: drawops.c,v 1.4 2005/07/30 11:38:51 cegger Exp $
******************************************************************************

   Display-libkgi: Channeling of GGI drawops through KGI batchop

   Copyright(C) 2001 by Brian S. Julin [bri@tull.umassp.edu]

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

#include <ggi/display/libkgi.h>

/* These should be in ggi/types.h as a convenience */
#define GGI_PT_DSTX	(GGI_PT_IS_COORD | GGI_PT_IN_X | GGI_PT_ROLE_START)
#define GGI_PT_DSTX2	(GGI_PT_IS_COORD | GGI_PT_IN_X | GGI_PT_ROLE_END)
#define GGI_PT_SRCX	(GGI_PT_DSTX | GGI_PT_IS_SRC)
#define GGI_PT_DSTW	(GGI_PT_IS_COORD | GGI_PT_IN_X | GGI_PT_ROLE_LEN)
#define GGI_PT_SRCW	(GGI_PT_DSTW | GGI_PT_IS_SRC)

#define GGI_PT_DSTY	(GGI_PT_IS_COORD | GGI_PT_IN_Y | GGI_PT_ROLE_START)
#define GGI_PT_DSTY2	(GGI_PT_IS_COORD | GGI_PT_IN_Y | GGI_PT_ROLE_END)
#define GGI_PT_SRCY	(GGI_PT_DSTY | GGI_PT_IS_SRC)
#define GGI_PT_DSTH	(GGI_PT_IS_COORD | GGI_PT_IN_Y | GGI_PT_ROLE_LEN)
#define GGI_PT_SRCH	(GGI_PT_DSTH | GGI_PT_IS_SRC)

#define GGI_PT_SRCADDR	(GGI_PT_IN_ADDR | GGI_PT_ROLE_START | GGI_PT_IS_SRC)
#define GGI_PT_DSTADDR	(GGI_PT_IN_ADDR | GGI_PT_ROLE_START)

#define GGI_PT_DSTSTRIDE (GGI_PT_IN_PIXEL | GGI_PT_ROLE_INCR)
#define GGI_PT_SRCSTRIDE (GGI_PT_DSTSTRIDE | GGI_PT_IS_SRC)



/* These are local to this file. */
#define BPADDR(idx) (&(block.align) + sizeof(int) * idx/4)
#define BPOFF(idx)  (((sizeof(int) * idx) % 4) * 8)
#define BPINIT(idx, pt) BPADDR(idx), BPOFF(idx), 0, sizeof(int), pt, 0, 0, {0}

#define KGI_BO (LIBKGI_PRIV(vis)->drawop_bo)
#define SEND_BO	bo_set_out_idx(((batchop_t)&bo), 0);	\
	bo_match(((batchop_t)&bo), KGI_BO, NULL);	\
        return bo_go(((batchop_t)&bo), KGI_BO, 1)

int GGI_libkgi_setorigin(struct ggi_visual *vis, int x, int y)
{
  return 0;
}

int GGI_libkgi_fillscreen(struct ggi_visual *vis) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_DSTX)	},
	  { BPINIT(1, GGI_PT_DSTY)	},
	  { BPINIT(2, GGI_PT_DSTW)	},
	  { BPINIT(3, GGI_PT_DSTH)	},
	  { BPINIT(4, GGI_PT_IS_OPCODE)	}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = 0;
	block.parms[1] = 0;
	block.parms[2] = LIBGGI_VIRTX(vis);
	block.parms[3] = LIBGGI_VIRTY(vis);
	block.parms[4] = GGI_OP_DRAWBOX;

	SEND_BO;
}

int GGI_libkgi_setdisplayframe(struct ggi_visual *vis, int num) 
{
  return 0;
}

int GGI_libkgi_setreadframe(struct ggi_visual *vis, int num) 
{
  return 0;
}

int GGI_libkgi_setwriteframe(struct ggi_visual *vis, int num) 
{
  return 0;
}

int GGI_libkgi_drawpixel(struct ggi_visual *vis,int x,int y) 
{
  return 0;
}

int GGI_libkgi_putpixel(struct ggi_visual *vis,int x,int y,ggi_pixel pixel) 
{
  return 0;
}

int GGI_libkgi_getpixel(struct ggi_visual *vis,int x,int y,ggi_pixel *pixel) 
{
  return 0;
}

int GGI_libkgi_putc(struct ggi_visual *vis,int x,int y,char c)
{
  return 0;
}

int GGI_libkgi_puts(struct ggi_visual *vis,int x,int y,const char *string)
{
  return 0;
}

int GGI_libkgi_getcharsize(struct ggi_visual *vis,int *width,int *height)
{
  return 0;
}

int GGI_libkgi_drawline(struct ggi_visual *vis,int x,int y,int xe,int ye)
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTX2)},
	  { BPINIT(3, GGI_PT_DSTY2)},
	  { BPINIT(4, GGI_PT_IS_OPCODE)}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = xe;
	block.parms[3] = ye;
	block.parms[4] = GGI_OP_DRAWLINE;

	SEND_BO;
}

int GGI_libkgi_drawhline(struct ggi_visual *vis,int x,int y,int w) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[4];
	} block;
	struct batchparm bp[4] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTW)},
	  { BPINIT(3, GGI_PT_IS_OPCODE)}
	};
	struct batchop bo  = {
	  bp, 4, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = GGI_OP_DRAWHLINE;

	SEND_BO;
}

int GGI_libkgi_puthline(struct ggi_visual *vis,int x,int y,int w,const void *buf) 

{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTW) },
	  { BPINIT(3, GGI_PT_IS_OPCODE)},
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_SRCADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = GGI_OP_PUTHLINE;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_gethline(struct ggi_visual *vis,int x,int y,int w,void *buf) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_SRCX) },
	  { BPINIT(1, GGI_PT_SRCY) },
	  { BPINIT(2, GGI_PT_SRCW) },
	  { BPINIT(3, GGI_PT_IS_OPCODE)},
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_DSTADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = GGI_OP_PUTHLINE | GGI_OP_GET;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_drawvline(struct ggi_visual *vis,int x,int y,int h) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[4];
	} block;
	struct batchparm bp[4] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTH) },
	  { BPINIT(3, GGI_PT_IS_OPCODE)}
	};
	struct batchop bo  = {
	  bp, 4, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = h;
	block.parms[3] = GGI_OP_DRAWVLINE;

	SEND_BO;
}

int GGI_libkgi_putvline(struct ggi_visual *vis,int x,int y,int h,const void *buf) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTH) },
	  { BPINIT(3, GGI_PT_IS_OPCODE)},
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_SRCADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = h;
	block.parms[3] = GGI_OP_PUTVLINE;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_getvline(struct ggi_visual *vis,int x,int y,int h,void *buf) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_SRCX) },
	  { BPINIT(1, GGI_PT_SRCY) },
	  { BPINIT(2, GGI_PT_SRCH) },
	  { BPINIT(3, GGI_PT_IS_OPCODE)},
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_DSTADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = h;
	block.parms[3] = GGI_OP_PUTVLINE | GGI_OP_GET;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_drawbox(struct ggi_visual *vis,int x,int y,int w,int h) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[5];
	} block;
	struct batchparm bp[5] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTW) },
	  { BPINIT(3, GGI_PT_DSTH) },
	  { BPINIT(4, GGI_PT_IS_OPCODE)}
	};
	struct batchop bo  = {
	  bp, 5, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = h;
	block.parms[4] = GGI_OP_DRAWBOX;

	SEND_BO;
}

int GGI_libkgi_putbox(struct ggi_visual *vis,int x,int y,int w,int h,const void *buf)
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[7];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[7] = {
	  { BPINIT(0, GGI_PT_DSTX) },
	  { BPINIT(1, GGI_PT_DSTY) },
	  { BPINIT(2, GGI_PT_DSTW) },
	  { BPINIT(3, GGI_PT_DSTH) },
	  { BPINIT(4, GGI_PT_SRCSTRIDE) },
	  { BPINIT(5, GGI_PT_IS_OPCODE) },
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_SRCADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 6, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = h;
	block.parms[4] = w;
	block.parms[5] = GGI_OP_PUTBOX;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_getbox(struct ggi_visual *vis,int x,int y,int w,int h,void *buf)
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[7];
	} block;
	union {
		uint64_t align;	/* Forces alignment (?) */
		void *parm;
	} block2;
	struct batchparm bp[7] = {
	  { BPINIT(0, GGI_PT_SRCX) },
	  { BPINIT(1, GGI_PT_SRCY) },
	  { BPINIT(2, GGI_PT_SRCW) },
	  { BPINIT(3, GGI_PT_SRCH) },
	  { BPINIT(4, GGI_PT_DSTSTRIDE) },
	  { BPINIT(5, GGI_PT_IS_OPCODE) },
	  { &block2.align, 0, 0, sizeof(void *), GGI_PT_DSTADDR,  0, 0, {0}}
	};
	struct batchop bo  = {
	  bp, 6, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = h;
	block.parms[4] = w;
	block.parms[5] = GGI_OP_PUTBOX | GGI_OP_GET;
	block2.parm = buf;

	SEND_BO;
}

int GGI_libkgi_copybox(struct ggi_visual *vis,int x,int y,int w,int h,
		       int nx,int ny) 
{
	union {
		uint64_t align;	/* Forces alignment (?) */
		int parms[7];
	} block;
	struct batchparm bp[7] = {
	  { BPINIT(0, GGI_PT_SRCX) },
	  { BPINIT(1, GGI_PT_SRCY) },
	  { BPINIT(2, GGI_PT_SRCW) },
	  { BPINIT(3, GGI_PT_SRCH) },
	  { BPINIT(2, GGI_PT_DSTX) },
	  { BPINIT(3, GGI_PT_DSTY) },
	  { BPINIT(4, GGI_PT_IS_OPCODE)}
	};
	struct batchop bo  = {
	  bp, 7, NULL, NULL, NULL, { 0 }, { 0 }
	};

	block.parms[0] = x;
	block.parms[1] = y;
	block.parms[2] = w;
	block.parms[3] = h;
	block.parms[4] = nx;
	block.parms[5] = ny;
	block.parms[6] = GGI_OP_DRAWBOX;

	SEND_BO;
}

int GGI_libkgi_crossblit(struct ggi_visual *src,int sx,int sy,int w,int h,
			 struct ggi_visual *dst,int dx,int dy) 
{
  return 0;
}

