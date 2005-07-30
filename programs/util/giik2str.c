/* $Id: giik2str.c,v 1.5 2005/07/30 08:43:02 soyt Exp $
******************************************************************************

   Conversion routine from GII sym/label to string.

   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/events.h>

#include <stdio.h>
#include <ctype.h>

#include "giik2str.h"

/* Hey, this function never claimed to be threadsafe... */
static char retbuffer[32]; /* More than enough to hold an hex 32bit int */

const char *giik2str(uint32 giik, int issym)
{
	switch (giik) {
	case GIIK_F0:	return "GIIK_F0";
	case GIIK_F1:	return "GIIK_F1";
	case GIIK_F2:	return "GIIK_F2";
	case GIIK_F3:	return "GIIK_F3";
	case GIIK_F4:	return "GIIK_F4";
	case GIIK_F5:	return "GIIK_F5";
	case GIIK_F6:	return "GIIK_F6";
	case GIIK_F7:	return "GIIK_F7";
	case GIIK_F8:	return "GIIK_F8";
	case GIIK_F9:	return "GIIK_F9";
	case GIIK_F10:	return "GIIK_F10";
	case GIIK_F11:	return "GIIK_F11";
	case GIIK_F12:	return "GIIK_F12";
	case GIIK_F13:	return "GIIK_F13";
	case GIIK_F14:	return "GIIK_F14";
	case GIIK_F15:	return "GIIK_F15";
	case GIIK_F16:	return "GIIK_F16";
	case GIIK_F17:	return "GIIK_F17";
	case GIIK_F18:	return "GIIK_F18";
	case GIIK_F19:	return "GIIK_F19";
	case GIIK_F20:	return "GIIK_F20";
	case GIIK_F21:	return "GIIK_F21";
	case GIIK_F22:	return "GIIK_F22";
	case GIIK_F23:	return "GIIK_F23";
	case GIIK_F24:	return "GIIK_F24";
	case GIIK_F25:	return "GIIK_F25";
	case GIIK_F26:	return "GIIK_F26";
	case GIIK_F27:	return "GIIK_F27";
	case GIIK_F28:	return "GIIK_F28";
	case GIIK_F29:	return "GIIK_F29";
	case GIIK_F30:	return "GIIK_F30";
	case GIIK_F31:	return "GIIK_F31";
	case GIIK_F32:	return "GIIK_F32";
	case GIIK_F33:	return "GIIK_F33";
	case GIIK_F34:	return "GIIK_F34";
	case GIIK_F35:	return "GIIK_F35";
	case GIIK_F36:	return "GIIK_F36";
	case GIIK_F37:	return "GIIK_F37";
	case GIIK_F38:	return "GIIK_F38";
	case GIIK_F39:	return "GIIK_F39";
	case GIIK_F40:	return "GIIK_F40";
	case GIIK_F41:	return "GIIK_F41";
	case GIIK_F42:	return "GIIK_F42";
	case GIIK_F43:	return "GIIK_F43";
	case GIIK_F44:	return "GIIK_F44";
	case GIIK_F45:	return "GIIK_F45";
	case GIIK_F46:	return "GIIK_F46";
	case GIIK_F47:	return "GIIK_F47";
	case GIIK_F48:	return "GIIK_F48";
	case GIIK_F49:	return "GIIK_F49";
	case GIIK_F50:	return "GIIK_F50";
	case GIIK_F51:	return "GIIK_F51";
	case GIIK_F52:	return "GIIK_F52";
	case GIIK_F53:	return "GIIK_F53";
	case GIIK_F54:	return "GIIK_F54";
	case GIIK_F55:	return "GIIK_F55";
	case GIIK_F56:	return "GIIK_F56";
	case GIIK_F57:	return "GIIK_F57";
	case GIIK_F58:	return "GIIK_F58";
	case GIIK_F59:	return "GIIK_F59";
	case GIIK_F60:	return "GIIK_F60";
	case GIIK_F61:	return "GIIK_F61";
	case GIIK_F62:	return "GIIK_F62";
	case GIIK_F63:	return "GIIK_F63";
	case GIIK_F64:	return "GIIK_F64";

	case GIIK_VOID:	return "GIIK_VOID";

	case GIIK_Enter:	return "GIIK_Enter";
	case GIIK_Delete:	return "GIIK_Delete";
	case GIIK_ScrollForw:	return "GIIK_ScrollForw";
	case GIIK_ScrollBack:	return "GIIK_ScrollBack";

	case GIIK_Break:	return "GIIK_Break";
	case GIIK_Boot:		return "GIIK_Boot";
	case GIIK_Compose:	return "GIIK_Compose";
	case GIIK_SAK:		return "GIIK_SAK";

	case GIIK_Undo:		return "GIIK_Undo";
	case GIIK_Redo:		return "GIIK_Redo";
	case GIIK_Menu:		return "GIIK_Menu";
	case GIIK_Cancel:	return "GIIK_Cancel";
	case GIIK_PrintScreen:	return "GIIK_PrintScreen";
	case GIIK_Execute:	return "GIIK_Execute";
	case GIIK_Find:		return "GIIK_Find";
	case GIIK_Begin:	return "GIIK_Begin";
	case GIIK_Clear:	return "GIIK_Clear";
	case GIIK_Insert:	return "GIIK_Insert";
	case GIIK_Select:	return "GIIK_Select";
	case GIIK_Macro:	return "GIIK_Macro";
	case GIIK_Help:		return "GIIK_Help";
	case GIIK_Do:		return "GIIK_Do";
	case GIIK_Pause:	return "GIIK_Pause";
	case GIIK_SysRq:	return "GIIK_SysRq";
	case GIIK_ModeSwitch:	return "GIIK_ModeSwitch";

	case GIIK_Up:		return "GIIK_Up";
	case GIIK_Down:		return "GIIK_Down";
	case GIIK_Left:		return "GIIK_Left";
	case GIIK_Right:	return "GIIK_Right";
	case GIIK_PageUp:	return "GIIK_PageUp";
	case GIIK_PageDown:	return "GIIK_PageDown";
	case GIIK_Home:		return "GIIK_Home";
	case GIIK_End:		return "GIIK_End";

	case GIIK_P0:	return "GIIK_P0";
	case GIIK_P1:	return "GIIK_P1";
	case GIIK_P2:	return "GIIK_P2";
	case GIIK_P3:	return "GIIK_P3";
	case GIIK_P4:	return "GIIK_P4";
	case GIIK_P5:	return "GIIK_P5";
	case GIIK_P6:	return "GIIK_P6";
	case GIIK_P7:	return "GIIK_P7";
	case GIIK_P8:	return "GIIK_P8";
	case GIIK_P9:	return "GIIK_P9";
	case GIIK_PA:	return "GIIK_PA";
	case GIIK_PB:	return "GIIK_PB";
	case GIIK_PC:	return "GIIK_PC";
	case GIIK_PD:	return "GIIK_PD";
	case GIIK_PE:	return "GIIK_PE";
	case GIIK_PF:	return "GIIK_PF";

	case GIIK_PPlus:	return "GIIK_PPlus";
	case GIIK_PMinus:	return "GIIK_PMinus";
	case GIIK_PAsterisk:	return "GIIK_PAsterisk";
	case GIIK_PSlash:	return "GIIK_PSlash";
	case GIIK_PEnter:	return "GIIK_PEnter";
	case GIIK_PPlusMinus:	return "GIIK_PPlusMinus";
	case GIIK_PParenLeft:	return "GIIK_PParenLeft";
	case GIIK_PParenRight:	return "GIIK_PParenRight";
	case GIIK_PSpace:	return "GIIK_PSpace";
	case GIIK_PTab:		return "GIIK_PTab";
	case GIIK_PBegin:	return "GIIK_PBegin";
	case GIIK_PEqual:	return "GIIK_PEqual";
	case GIIK_PDecimal:	return "GIIK_PDecimal";
	case GIIK_PSeparator:	return "GIIK_PSeparator";

	case GIIK_PF1:	return "GIIK_PF1";
	case GIIK_PF2:	return "GIIK_PF2";
	case GIIK_PF3:	return "GIIK_PF3";
	case GIIK_PF4:	return "GIIK_PF4";
	case GIIK_PF5:	return "GIIK_PF5";
	case GIIK_PF6:	return "GIIK_PF6";
	case GIIK_PF7:	return "GIIK_PF7";
	case GIIK_PF8:	return "GIIK_PF8";
	case GIIK_PF9:	return "GIIK_PF9";

	case GIIK_Shift:
		if (issym)	return "GIIK_Shift";
		else		return "GIIK_ShiftL";
	case GIIK_Ctrl:
		if (issym)	return "GIIK_Ctrl";
		else		return "GIIK_CtrlL";
	case GIIK_Alt:
		if (issym)	return "GIIK_Alt";
		else		return "GIIK_AltL";
	case GIIK_Meta:
		if (issym)	return "GIIK_Meta";
		else		return "GIIK_MetaL";
	case GIIK_Super:
		if (issym)	return "GIIK_Super";
		else		return "GIIK_SuperL";
	case GIIK_Hyper:
		if (issym)	return "GIIK_Hyper";
		else		return "GIIK_HyperL";

	case GIIK_ShiftR:	return "GIIK_ShiftR";
	case GIIK_CtrlR:	return "GIIK_CtrlR";
	case GIIK_AltR:		return "GIIK_AltR";
	case GIIK_MetaR:	return "GIIK_MetaR";
	case GIIK_SuperR:	return "GIIK_SuperR";
	case GIIK_HyperR:	return "GIIK_HyperR";

	case GIIK_AltGr:	return "GIIK_AltGr";
	case GIIK_Caps:		return "GIIK_Caps";
	case GIIK_Num:		return "GIIK_Num";
	case GIIK_Scroll:	return "GIIK_Scroll";

	case GIIK_ShiftLock:	return "GIIK_ShiftLock";
	case GIIK_CtrlLock:	return "GIIK_CtrlLock";
	case GIIK_AltLock:	return "GIIK_AltLock";
	case GIIK_MetaLock:	return "GIIK_MetaLock";
	case GIIK_SuperLock:	return "GIIK_SuperLock";
	case GIIK_HyperLock:	return "GIIK_HyperLock";
	case GIIK_AltGrLock:	return "GIIK_AltGrLock";
	case GIIK_CapsLock:	return "GIIK_CapsLock";
	case GIIK_NumLock:	return "GIIK_NumLock";
	case GIIK_ScrollLock:	return "GIIK_ScrollLock";

	case GIIK_NIL:		return "GIIK_NIL";

	case GIIUC_Nul:		return "GIIUC_Nul";
	case GIIUC_BackSpace:	return "GIIUC_BackSpace";
	case GIIUC_Tab:		return "GIIUC_Tab";
	case GIIUC_Linefeed:	return "GIIUC_Linefeed";
	case GIIUC_Escape:	return "GIIUC_Escape";

#ifdef GIIK_DeadRing
	case GIIK_DeadRing:		return "GIIK_DeadRing";
	case GIIK_DeadCaron:		return "GIIK_DeadCaron";
	case GIIK_DeadOgonek:		return "GIIK_DeadOgonek";
	case GIIK_DeadIota:		return "GIIK_DeadIota";
	case GIIK_DeadDoubleAcute:	return "GIIK_DeadDoubleAcute";
	case GIIK_DeadBreve:		return "GIIK_DeadBreve";
	case GIIK_DeadAboveDot:		return "GIIK_DeadAboveDot";
	case GIIK_DeadBelowDot:		return "GIIK_DeadBelowDot";
	case GIIK_DeadVoicedSound:	return "GIIK_DeadVoicedSound";
	case GIIK_DeadSemiVoicedSound:	return "GIIK_DeadSemiVoicedSound";
	case GIIK_DeadAcute:		return "GIIK_DeadAcute";
	case GIIK_DeadCedilla:		return "GIIK_DeadCedilla";
	case GIIK_DeadCircumflex:	return "GIIK_DeadCircumflex";
	case GIIK_DeadDiaeresis:	return "GIIK_DeadDiaeresis";
	case GIIK_DeadGrave:		return "GIIK_DeadGrave";
	case GIIK_DeadTilde:		return "GIIK_DeadTilde";
	case GIIK_DeadMacron:		return "GIIK_DeadMacron";
#endif
		
	default:
		if (giik >= 32 && giik < 256) {
			/* Handle sym/label here */
			if (!issym) giik = toupper((uint8)giik);
			switch (giik) {
			case GIIUC_Space:	return "GIIUC_Space";
			case GIIUC_Exclamation:	return "GIIUC_Exclamation";
			case GIIUC_DoubleQuote:	return "GIIUC_DoubleQuote";
			case GIIUC_NumberSign:	return "GIIUC_NumberSign";
			case GIIUC_Dollar:	return "GIIUC_Dollar";
			case GIIUC_Percent:	return "GIIUC_Percent";
			case GIIUC_Ampersand:	return "GIIUC_Ampersand";
			case GIIUC_Apostrophe:	return "GIIUC_Apostrophe";
			case GIIUC_ParenLeft:	return "GIIUC_ParenLeft";
			case GIIUC_ParenRight:	return "GIIUC_ParenRight";
			case GIIUC_Asterisk:	return "GIIUC_Asterisk";
			case GIIUC_Plus:	return "GIIUC_Plus";
			case GIIUC_Comma:	return "GIIUC_Comma";
			case GIIUC_Minus:	return "GIIUC_Minus";
			case GIIUC_Period:	return "GIIUC_Period";
			case GIIUC_Slash:	return "GIIUC_Slash";
			case GIIUC_0:		return "GIIUC_0";
			case GIIUC_1:		return "GIIUC_1";
			case GIIUC_2:		return "GIIUC_2";
			case GIIUC_3:		return "GIIUC_3";
			case GIIUC_4:		return "GIIUC_4";
			case GIIUC_5:		return "GIIUC_5";
			case GIIUC_6:		return "GIIUC_6";
			case GIIUC_7:		return "GIIUC_7";
			case GIIUC_8:		return "GIIUC_8";
			case GIIUC_9:		return "GIIUC_9";
			case GIIUC_Colon:	return "GIIUC_Colon";
			case GIIUC_Semicolon:	return "GIIUC_Semicolon";
			case GIIUC_Less:	return "GIIUC_Less";
			case GIIUC_Equal:	return "GIIUC_Equal";
			case GIIUC_Greater:	return "GIIUC_Greater";
			case GIIUC_Question:	return "GIIUC_Question";
			case GIIUC_At:	return "GIIUC_At";
			case GIIUC_A:	return "GIIUC_A";
			case GIIUC_B:	return "GIIUC_B";
			case GIIUC_C:	return "GIIUC_C";
			case GIIUC_D:	return "GIIUC_D";
			case GIIUC_E:	return "GIIUC_E";
			case GIIUC_F:	return "GIIUC_F";
			case GIIUC_G:	return "GIIUC_G";
			case GIIUC_H:	return "GIIUC_H";
			case GIIUC_I:	return "GIIUC_I";
			case GIIUC_J:	return "GIIUC_J";
			case GIIUC_K:	return "GIIUC_K";
			case GIIUC_L:	return "GIIUC_L";
			case GIIUC_M:	return "GIIUC_M";
			case GIIUC_N:	return "GIIUC_N";
			case GIIUC_O:	return "GIIUC_O";
			case GIIUC_P:	return "GIIUC_P";
			case GIIUC_Q:	return "GIIUC_Q";
			case GIIUC_R:	return "GIIUC_R";
			case GIIUC_S:	return "GIIUC_S";
			case GIIUC_T:	return "GIIUC_T";
			case GIIUC_U:	return "GIIUC_U";
			case GIIUC_V:	return "GIIUC_V";
			case GIIUC_W:	return "GIIUC_W";
			case GIIUC_X:	return "GIIUC_X";
			case GIIUC_Y:	return "GIIUC_Y";
			case GIIUC_Z:	return "GIIUC_Z";
			case GIIUC_BracketLeft:	return "GIIUC_BracketLeft";
			case GIIUC_BackSlash:	return "GIIUC_BackSlash";
			case GIIUC_BracketRight:return "GIIUC_BracketRight";
			case GIIUC_Circumflex:	return "GIIUC_Circumflex";
			case GIIUC_Underscore:	return "GIIUC_Underscore";
			case GIIUC_Grave:	return "GIIUC_Grave";
			case GIIUC_a:	return "GIIUC_a";
			case GIIUC_b:	return "GIIUC_b";
			case GIIUC_c:	return "GIIUC_c";
			case GIIUC_d:	return "GIIUC_d";
			case GIIUC_e:	return "GIIUC_e";
			case GIIUC_f:	return "GIIUC_f";
			case GIIUC_g:	return "GIIUC_g";
			case GIIUC_h:	return "GIIUC_h";
			case GIIUC_i:	return "GIIUC_i";
			case GIIUC_j:	return "GIIUC_j";
			case GIIUC_k:	return "GIIUC_k";
			case GIIUC_l:	return "GIIUC_l";
			case GIIUC_m:	return "GIIUC_m";
			case GIIUC_n:	return "GIIUC_n";
			case GIIUC_o:	return "GIIUC_o";
			case GIIUC_p:	return "GIIUC_p";
			case GIIUC_q:	return "GIIUC_q";
			case GIIUC_r:	return "GIIUC_r";
			case GIIUC_s:	return "GIIUC_s";
			case GIIUC_t:	return "GIIUC_t";
			case GIIUC_u:	return "GIIUC_u";
			case GIIUC_v:	return "GIIUC_v";
			case GIIUC_w:	return "GIIUC_w";
			case GIIUC_x:	return "GIIUC_x";
			case GIIUC_y:	return "GIIUC_y";
			case GIIUC_z:	return "GIIUC_z";
			case GIIUC_BraceLeft:	return "GIIUC_BraceLeft";
			case GIIUC_Bar:		return "GIIUC_Bar";
			case GIIUC_BraceRight:	return "GIIUC_BraceRight";
			case GIIUC_Tilde:	return "GIIUC_Tilde";

			case GIIUC_NoBreakSpace:return "GIIUC_NoBreakSpace";
			case GIIUC_ExclamDown:	return "GIIUC_ExclamDown";
			case GIIUC_Cent:	return "GIIUC_Cent";
			case GIIUC_Sterling:	return "GIIUC_Sterling";
			case GIIUC_Currency:	return "GIIUC_Currency";
			case GIIUC_Yen:		return "GIIUC_Yen";
			case GIIUC_BrokenBar:	return "GIIUC_BrokenBar";
			case GIIUC_Section:	return "GIIUC_Section";
			case GIIUC_Diaeresis:	return "GIIUC_Diaeresis";
			case GIIUC_Copyright:	return "GIIUC_Copyright";
			case GIIUC_OrdFeminine:	return "GIIUC_OrdFeminine";
			case GIIUC_GuillemotLeft:return "GIIUC_GuillemotLeft";
			case GIIUC_NotSign:	return "GIIUC_NotSign";
			case GIIUC_SoftHyphen:	return "GIIUC_SoftHyphen";
			case GIIUC_Registered:	return "GIIUC_Registered";
			case GIIUC_Macron:	return "GIIUC_Macron";
			case GIIUC_Degree:	return "GIIUC_Degree";
			case GIIUC_PlusMinus:	return "GIIUC_PlusMinus";
			case GIIUC_TwoSuperior:	return "GIIUC_TwoSuperior";
			case GIIUC_ThreeSuperior:return "GIIUC_ThreeSuperior";
			case GIIUC_Acute:	return "GIIUC_Acute";
			case GIIUC_Mu:		return "GIIUC_Mu";
			case GIIUC_Paragraph:	return "GIIUC_Paragraph";
			case GIIUC_PeriodCentered:return "GIIUC_PeriodCentered";
			case GIIUC_Cedilla:	return "GIIUC_Cedilla";
			case GIIUC_OneSuperior:	return "GIIUC_OneSuperior";
			case GIIUC_mKuline:	return "GIIUC_mKuline";
			case GIIUC_GuillemotRight:return "GIIUC_GuillemotRight";
			case GIIUC_OneQuarter:	return "GIIUC_OneQuarter";
			case GIIUC_OneHalf:	return "GIIUC_OneHalf";
			case GIIUC_ThreeQuarters:return "GIIUC_ThreeQuarters";
			case GIIUC_QuestionDown:return "GIIUC_QuestionDown";
			case GIIUC_Agrave:	return "GIIUC_Agrave";
			case GIIUC_Aacute:	return "GIIUC_Aacute";
			case GIIUC_Acircumflex:	return "GIIUC_Acircumflex";
			case GIIUC_Atilde:	return "GIIUC_Atilde";
			case GIIUC_Adiaeresis:	return "GIIUC_Adiaeresis";
			case GIIUC_Aring:	return "GIIUC_Aring";
			case GIIUC_AE:		return "GIIUC_AE";
			case GIIUC_Ccedilla:	return "GIIUC_Ccedilla";
			case GIIUC_Egrave:	return "GIIUC_Egrave";
			case GIIUC_Eacute:	return "GIIUC_Eacute";
			case GIIUC_Ecircumflex:	return "GIIUC_Ecircumflex";
			case GIIUC_Ediaeresis:	return "GIIUC_Ediaeresis";
			case GIIUC_Igrave:	return "GIIUC_Igrave";
			case GIIUC_Iacute:	return "GIIUC_Iacute";
			case GIIUC_Icircumflex:	return "GIIUC_Icircumflex";
			case GIIUC_Idiaeresis:	return "GIIUC_Idiaeresis";
			case GIIUC_ETH:		return "GIIUC_ETH";
			case GIIUC_Ntilde:	return "GIIUC_Ntilde";
			case GIIUC_Ograve:	return "GIIUC_Ograve";
			case GIIUC_Oacute:	return "GIIUC_Oacute";
			case GIIUC_Ocircumflex:	return "GIIUC_Ocircumflex";
			case GIIUC_Otilde:	return "GIIUC_Otilde";
			case GIIUC_Odiaeresis:	return "GIIUC_Odiaeresis";
			case GIIUC_Multiply:	return "GIIUC_Multiply";
			case GIIUC_Ooblique:	return "GIIUC_Ooblique";
			case GIIUC_Ugrave:	return "GIIUC_Ugrave";
			case GIIUC_Uacute:	return "GIIUC_Uacute";
			case GIIUC_Ucircumflex:	return "GIIUC_Ucircumflex";
			case GIIUC_Udiaeresis:	return "GIIUC_Udiaeresis";
			case GIIUC_Yacute:	return "GIIUC_Yacute";
			case GIIUC_THORN:	return "GIIUC_THORN";
			case GIIUC_ssharp:	return "GIIUC_ssharp";
			case GIIUC_agrave:	return "GIIUC_agrave";
			case GIIUC_aacute:	return "GIIUC_aacute";
			case GIIUC_acircumflex:	return "GIIUC_acircumflex";
			case GIIUC_atilde:	return "GIIUC_atilde";
			case GIIUC_adiaeresis:	return "GIIUC_adiaeresis";
			case GIIUC_aring:	return "GIIUC_aring";
			case GIIUC_ae:		return "GIIUC_ae";
			case GIIUC_ccedilla:	return "GIIUC_ccedilla";
			case GIIUC_egrave:	return "GIIUC_egrave";
			case GIIUC_eacute:	return "GIIUC_eacute";
			case GIIUC_ecircumflex:	return "GIIUC_ecircumflex";
			case GIIUC_ediaeresis:	return "GIIUC_ediaeresis";
			case GIIUC_igrave:	return "GIIUC_igrave";
			case GIIUC_iacute:	return "GIIUC_iacute";
			case GIIUC_icircumflex:	return "GIIUC_icircumflex";
			case GIIUC_idiaeresis:	return "GIIUC_idiaeresis";
			case GIIUC_eth:		return "GIIUC_eth";
			case GIIUC_ntilde:	return "GIIUC_ntilde";
			case GIIUC_ograve:	return "GIIUC_ograve";
			case GIIUC_oacute:	return "GIIUC_oacute";
			case GIIUC_ocircumflex:	return "GIIUC_ocircumflex";
			case GIIUC_otilde:	return "GIIUC_otilde";
			case GIIUC_odiaeresis:	return "GIIUC_odiaeresis";
			case GIIUC_Division:	return "GIIUC_Division";
			case GIIUC_oslash:	return "GIIUC_oslash";
			case GIIUC_ugrave:	return "GIIUC_ugrave";
			case GIIUC_uacute:	return "GIIUC_uacute";
			case GIIUC_ucircumflex:	return "GIIUC_ucircumflex";
			case GIIUC_udiaeresis:	return "GIIUC_udiaeresis";
			case GIIUC_yacute:	return "GIIUC_yacute";
			case GIIUC_thorn:	return "GIIUC_thorn";
			case GIIUC_ydiaeresis:	return "GIIUC_ydiaeresis";
			}
		}

		/* Return hex integer string */
		sprintf(retbuffer, "0x%04x", giik);
		return retbuffer;
	}
}
