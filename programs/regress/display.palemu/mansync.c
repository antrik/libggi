/* $Id: mansync.c,v 1.2 2004/10/10 22:14:36 cegger Exp $
******************************************************************************

   This is a regression-test for LibGGI display-palemu - mansync usage.

   Written in 2004 by Christoph Egger

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/


#include "config.h"
#include <ggi/internal/internal.h>
#include <ggi/display/palemu.h>
#include <ggi/ggi.h>


#define DISPLAYSTR	"display-palemu"

#include "../display.mansync/mansync.inc.c"
