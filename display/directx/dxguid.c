/* $Id: dxguid.c,v 1.1 2004/11/02 08:52:33 pekberg Exp $
******************************************************************************

   Used DirectX IIDs and GUIDs

   Copyright (C) 2004      Peter Ekberg		[peda@lysator.liu.se]

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

/* libtool fails to link a dll against libdxguid.a which
 * is what we really want to do here. When libtool can do
 * that, kill this file and add -ldxguid to the link
 * command line.
 * IMHO, this is a horrid workaround. But it works, and we
 * get a dll instead of a static lib.
 */

typedef struct _IID
{
    unsigned long  x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

typedef IID GUID;

const IID  IID_IDirectDraw2 = {0xb3a6f3e0,0x2b43,0x11cf,{0xa2,0xde,0x00,0xaa,0x00,0xb9,0x33,0x56}};
