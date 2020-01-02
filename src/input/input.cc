#include <config.hh>

#include <deque>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <iostream>
#include <termios.h>

#include <limits>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>

using namespace ncpp;

static int dimy, dimx;
static NotCurses *nc;

static constexpr char32_t INVALID_CHAR = std::numeric_limits<char32_t>::max ();

// return the string version of a special composed key
const char* nckeystr (char32_t spkey)
{
	switch (spkey) { // FIXME
		case NCKey::Resize:
			nc->resize (&dimy, &dimx);
			return "resize event";
		case NCKey::Invalid: return "invalid";
		case NCKey::Left:    return "left";
		case NCKey::Up:      return "up";
		case NCKey::Right:   return "right";
		case NCKey::Down:    return "down";
		case NCKey::Ins:     return "insert";
		case NCKey::Del:     return "delete";
		case NCKey::PgDown:  return "pgdown";
		case NCKey::PgUp:    return "pgup";
		case NCKey::Home:    return "home";
		case NCKey::End:     return "end";
		case NCKey::F00:     return "F0";
		case NCKey::F01:     return "F1";
		case NCKey::F02:     return "F2";
		case NCKey::F03:     return "F3";
		case NCKey::F04:     return "F4";
		case NCKey::F05:     return "F5";
		case NCKey::F06:     return "F6";
		case NCKey::F07:     return "F7";
		case NCKey::F08:     return "F8";
		case NCKey::F09:     return "F9";
		case NCKey::F10:     return "F10";
		case NCKey::F11:     return "F11";
		case NCKey::F12:     return "F12";
		case NCKey::F13:     return "F13";
		case NCKey::F14:     return "F14";
		case NCKey::F15:     return "F15";
		case NCKey::F16:     return "F16";
		case NCKey::F17:     return "F17";
		case NCKey::F18:     return "F18";
		case NCKey::F19:     return "F19";
		case NCKey::F20:     return "F20";
		case NCKey::F21:     return "F21";
		case NCKey::F22:     return "F22";
		case NCKey::F23:     return "F23";
		case NCKey::F24:     return "F24";
		case NCKey::F25:     return "F25";
		case NCKey::F26:     return "F26";
		case NCKey::F27:     return "F27";
		case NCKey::F28:     return "F28";
		case NCKey::F29:     return "F29";
		case NCKey::F30:     return "F30";
		case NCKey::Backspace: return "backspace";
		case NCKey::Center:  return "center";
		case NCKey::Enter:   return "enter";
		case NCKey::CLS:     return "clear";
		case NCKey::DLeft:   return "down+left";
		case NCKey::DRight:  return "down+right";
		case NCKey::ULeft:   return "up+left";
		case NCKey::URight:  return "up+right";
		case NCKey::Begin:   return "begin";
		case NCKey::Cancel:  return "cancel";
		case NCKey::Close:   return "close";
		case NCKey::Command: return "command";
		case NCKey::Copy:    return "copy";
		case NCKey::Exit:    return "exit";
		case NCKey::Print:   return "print";
		case NCKey::Refresh: return "refresh";
		case NCKey::Button1: return "mouse (button 1 pressed)";
		case NCKey::Button2: return "mouse (button 2 pressed)";
		case NCKey::Button3: return "mouse (button 3 pressed)";
		case NCKey::Button4: return "mouse (button 4 pressed)";
		case NCKey::Button5: return "mouse (button 5 pressed)";
		case NCKey::Button6: return "mouse (button 6 pressed)";
		case NCKey::Button7: return "mouse (button 7 pressed)";
		case NCKey::Button8: return "mouse (button 8 pressed)";
		case NCKey::Button9: return "mouse (button 9 pressed)";
		case NCKey::Button10: return "mouse (button 10 pressed)";
		case NCKey::Button11: return "mouse (button 11 pressed)";
		case NCKey::Release: return "mouse (button released)";
		default:            return "unknown";
	}
}

// Print the utf8 Control Pictures for otherwise unprintable ASCII
char32_t printutf8 (char32_t kp)
{
	if (kp <= 27 && kp < INVALID_CHAR) {
		return 0x2400 + kp;
	}

	return kp;
}

// Dim all text on the plane by the same amount. This will stack for
// older text, and thus clearly indicate the current output.
static bool
dim_rows (Plane* n)
{
	int y, x;
	Cell c;
	for (y = 2 ; y < dimy ; ++y) {
		for (x = 0 ; x < dimx ; ++x) {
			if (n->get_at (y, x, c) < 0) {
				n->release (c);
				return false;
			}

			unsigned r, g, b;
			c.get_fg_rgb (&r, &g, &b);
			r -= r / 32;
			g -= g / 32;
			b -= b / 32;
			if (r > 247){ r = 0; }
			if (g > 247){ g = 0; }
			if (b > 247){ b = 0; }
			if (!c.set_fg_rgb (r, g, b)) {
				n->release (c);
				return false;
			}

			if (n->putc (y, x, c) < 0) {
				n->release (c);
				return false;
			}

			if (c.is_double_wide ()) {
				++x;
			}
		}
	}

	n->release (c);
	return true;
}

int main (void)
{
	if (setlocale(LC_ALL, "") == nullptr) {
		return EXIT_FAILURE;
	}

	nc = &NotCurses::get_instance ();
	NotCurses::default_notcurses_options.clear_screen_start = true;
	if (!nc->init ()) {
		return EXIT_FAILURE;;
	}

	Plane* n = nc->get_stdplane ();
	nc->get_term_dim (&dimy, &dimx);
	n->set_fg (0);
	n->set_bg (0xbb64bb);
	n->styles_set (CellStyle::Underline);
	if (n->putstr (0, NCAlign::Center, "mash keys, yo. give that mouse some waggle! ctrl+d exits.") <= 0) {
		return EXIT_FAILURE;
	}

	n->styles_off (CellStyle::Underline);
	n->set_bg_default ();
	nc->render ();

	int y = 2;
	std::deque<char32_t> cells;
	char32_t r;
	if (!nc->mouse_enable ()) {
		return EXIT_FAILURE;
	}

	ncinput ni;
	while (errno = 0, (r = nc->getc (true, &ni)) < INVALID_CHAR) {
		if (r == 0) { // interrupted by signal
			continue;
		}

		if (r == CEOT) {
			return EXIT_SUCCESS;
		}

		if (!n->cursor_move (y, 0)) {
			break;
		}

		n->set_fg_rgb (0xd0, 0xd0, 0xd0);
		n->printf ("%c%c%c ", ni.alt ? 'A' : 'a', ni.ctrl ? 'C' : 'c', ni.shift ? 'S' : 's');

		if (r < 0x80) {
			n->set_fg_rgb (128, 250, 64);
			if (n->printf ("ASCII: [0x%02x (%03d)] '%lc'", r, r, iswprint(r) ? r : printutf8(r)) < 0) {
				break;
			}
		} else {
			if (nckey_supppuab_p (r)) {
				n->set_fg_rgb (250, 64, 128);
				if (n->printf ("Special: [0x%02x (%02d)] '%s'", r, r, nckeystr(r)) < 0) {
					break;
				}
				if (NCKey::IsMouse (r)) {
					if (n->printf (" x: %d y: %d", ni.x, ni.y) < 0) {
						break;
					}
				}
			} else {
				n->set_fg_rgb (64, 128, 250);
				n->printf ("Unicode: [0x%08x] '%lc'", r, r);
			}
		}

		if (!dim_rows (n)) {
			break;
		}

		if (!nc->render ()) {
			break;
		}

		if(++y >= dimy - 2) { // leave a blank line at the bottom
			y = 2;            // and at the top
		}

		while (cells.size () >= dimy - 3u){
			cells.pop_back ();
		}
		cells.push_front (r);
	}

	int e = errno;
	nc->mouse_disable ();
	if (r == INVALID_CHAR && e){
		std::cerr << "Error reading from terminal (" << strerror(e) << "?)\n";
	}
	return EXIT_FAILURE;
}
