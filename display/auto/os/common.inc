/* $Id: common.inc,v 1.1 2004/01/29 13:49:33 cegger Exp $
******************************************************************************

   Auto target for GGI - OS implementation

   Copyright (C) 2004 Christoph Egger

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


/* static struct ggi_auto_Target targetList[]
 * 
 * must be declared before inclusion of this file.
 */


#define NUM_TARGETS	(sizeof(targetList)/sizeof(struct ggi_auto_Target))


int OS_createTargetInfo(ggi_auto_priv *priv)
{
	priv->target = (struct ggi_auto_Target *)malloc(sizeof(targetList));
	if (!priv->target) return GGI_ENOMEM;

	memcpy(priv->target, targetList, sizeof(targetList));
	priv->num_targets = NUM_TARGETS;
	return GGI_OK;
}


int OS_freeTargetInfo(ggi_auto_priv *priv)
{
	free(priv->target);
	priv->target = NULL;
	priv->num_targets = 0;
	return GGI_OK;
}
