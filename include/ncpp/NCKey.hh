#ifndef __NCPP_NCKEY_HH
#define __NCPP_NCKEY_HH

#include <cstdint>
#include <notcurses.h>

namespace ncpp
{
	struct NCKey
	{
        static constexpr char32_t Invalid   = NCKEY_INVALID;
        static constexpr char32_t Resize    = NCKEY_RESIZE;
        static constexpr char32_t Up        = NCKEY_UP;
        static constexpr char32_t Right     = NCKEY_RIGHT;
        static constexpr char32_t Down      = NCKEY_DOWN;
        static constexpr char32_t Left      = NCKEY_LEFT;
        static constexpr char32_t Ins       = NCKEY_INS;
        static constexpr char32_t Del       = NCKEY_DEL;
        static constexpr char32_t Backspace = NCKEY_BACKSPACE;
        static constexpr char32_t PgDown    = NCKEY_PGDOWN;
        static constexpr char32_t PgUp      = NCKEY_PGUP;
        static constexpr char32_t Home      = NCKEY_HOME;
        static constexpr char32_t End       = NCKEY_END;
        static constexpr char32_t F00       = NCKEY_F00;
        static constexpr char32_t F01       = NCKEY_F01;
        static constexpr char32_t F02       = NCKEY_F02;
        static constexpr char32_t F03       = NCKEY_F03;
        static constexpr char32_t F04       = NCKEY_F04;
        static constexpr char32_t F05       = NCKEY_F05;
        static constexpr char32_t F06       = NCKEY_F06;
        static constexpr char32_t F07       = NCKEY_F07;
        static constexpr char32_t F08       = NCKEY_F08;
        static constexpr char32_t F09       = NCKEY_F09;
        static constexpr char32_t F10       = NCKEY_F10;
        static constexpr char32_t F11       = NCKEY_F11;
        static constexpr char32_t F12       = NCKEY_F12;
        static constexpr char32_t F13       = NCKEY_F13;
        static constexpr char32_t F14       = NCKEY_F14;
        static constexpr char32_t F15       = NCKEY_F15;
        static constexpr char32_t F16       = NCKEY_F16;
        static constexpr char32_t F17       = NCKEY_F17;
        static constexpr char32_t F18       = NCKEY_F18;
        static constexpr char32_t F19       = NCKEY_F19;
        static constexpr char32_t F20       = NCKEY_F20;
        static constexpr char32_t F21       = NCKEY_F21;
        static constexpr char32_t F22       = NCKEY_F22;
        static constexpr char32_t F23       = NCKEY_F23;
        static constexpr char32_t F24       = NCKEY_F24;
        static constexpr char32_t F25       = NCKEY_F25;
        static constexpr char32_t F26       = NCKEY_F26;
        static constexpr char32_t F27       = NCKEY_F27;
        static constexpr char32_t F28       = NCKEY_F28;
        static constexpr char32_t F29       = NCKEY_F29;
        static constexpr char32_t F30       = NCKEY_F30;
        static constexpr char32_t Enter     = NCKEY_ENTER;
        static constexpr char32_t CLS       = NCKEY_CLS;
        static constexpr char32_t DLeft     = NCKEY_DLEFT;
        static constexpr char32_t DRight    = NCKEY_DRIGHT;
        static constexpr char32_t ULeft     = NCKEY_ULEFT;
        static constexpr char32_t URight    = NCKEY_URIGHT;
        static constexpr char32_t Center    = NCKEY_CENTER;
        static constexpr char32_t Begin     = NCKEY_BEGIN;
        static constexpr char32_t Cancel    = NCKEY_CANCEL;
        static constexpr char32_t Close     = NCKEY_CLOSE;
        static constexpr char32_t Command   = NCKEY_COMMAND;
        static constexpr char32_t Copy      = NCKEY_COPY;
        static constexpr char32_t Exit      = NCKEY_EXIT;
        static constexpr char32_t Print     = NCKEY_PRINT;
        static constexpr char32_t Refresh   = NCKEY_REFRESH;
		static constexpr char32_t Button1   = NCKEY_BUTTON1;
		static constexpr char32_t Button2   = NCKEY_BUTTON2;
		static constexpr char32_t Button3   = NCKEY_BUTTON3;
		static constexpr char32_t Button4   = NCKEY_BUTTON4;
		static constexpr char32_t Button5   = NCKEY_BUTTON5;
		static constexpr char32_t Button6   = NCKEY_BUTTON6;
		static constexpr char32_t Button7   = NCKEY_BUTTON7;
		static constexpr char32_t Button8   = NCKEY_BUTTON8;
		static constexpr char32_t Button9   = NCKEY_BUTTON9;
		static constexpr char32_t Button10  = NCKEY_BUTTON10;
		static constexpr char32_t Button11  = NCKEY_BUTTON11;
		static constexpr char32_t Release   = NCKEY_RELEASE;

		static bool IsMouse (char32_t ch) noexcept
		{
			return nckey_mouse_p (ch);
		}

		static bool IsSuppUAB (char32_t ch) noexcept
		{
			return wchar_supppuab_p (ch);
		}
	};
}
#endif
