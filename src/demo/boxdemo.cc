#include <config.hh>

#include <memory>
#include <cstdlib>
#include <unistd.h>

#include <ncpp/NotCurses.hh>

#include "demo.hh"

using namespace ncpp;

constexpr int ITERATIONS = 10;

bool box_demo (NotCurses &nc)
{
	const int64_t totalns = timespec_to_ns (&demodelay);
	struct timespec iterdelay;

	timespec_div (&demodelay, ITERATIONS, &iterdelay);

	std::unique_ptr<Plane> n (nc.get_stdplane ());
	n->erase ();

	Cell ul, ll;
	Cell lr, ur;
	Cell hl, vl;
	if (!n->load_double_box (0, 0, ul, ur, ll, lr, hl, vl)) {
		return false;
	}

	struct timespec start, now;
	clock_gettime (CLOCK_MONOTONIC_RAW, &start);

	int zbonus = 40;
	int zbonusdelta = 20;
	int ylen, xlen;
	n->get_dim (&ylen, &xlen);

	// target grid is 7x7
	const int targx = 7;
	const int targy = 7;
	int ytargbase = (ylen - targy) / 2;
	Cell c(' ');
	c.set_bg_default ();
	n->set_default (c);
	n->release (c);

	n->set_fg_rgb (180, 40, 180);
	n->set_bg_default ();
	if (n->putstr (ytargbase++, NCAlign::Center, "┏━━┳━━┓") < 0) {
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┃┌─╂─┐┃") < 0) {
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┃│╲╿╱│┃") < 0){
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┣┿╾╳╼┿┫") < 0){
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┃│╱╽╲│┃") < 0){
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┃└─╂─┘┃") < 0){
		return false;
	}

	if (n->putstr (ytargbase++, NCAlign::Center, "┗━━┻━━┛") < 0){
		return false;
	}

	do {
		int y = 0, x = 0;
		n->get_dim (&ylen, &xlen);
		while (ylen - y >= targy && xlen - x >= targx) {
			ul.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			ul.set_bg_rgb (20, zbonus, 20);
			ur.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			ur.set_bg_rgb (20, zbonus, 20);
			hl.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			hl.set_bg_rgb (20, zbonus, 20);
			ll.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			ll.set_bg_rgb (20, zbonus, 20);
			lr.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			lr.set_bg_rgb (20, zbonus, 20);
			vl.set_fg_rgb (107 - (y * 2), zbonus, 107 + (y * 2));
			vl.set_bg_rgb (20, zbonus, 20);

			if (!n->cursor_move (y, x)) {
				return false;
			}

			if (!n->box_sized (ul, ur, ll, lr, hl, vl, ylen, xlen, NCBox::GradLeft | NCBox::GradBottom | NCBox::GradRight | NCBox::GradTop)) {
				return false;
			}
			ylen -= 2;
			xlen -= 2;
			++y;
			++x;
		}

		if (!nc.render ()) {
			return false;
		}

		nanosleep (&iterdelay, nullptr);
		clock_gettime (CLOCK_MONOTONIC_RAW, &now);

		if ((zbonus += zbonusdelta > 255) || zbonus < 0) {
			zbonusdelta = -zbonusdelta;
			zbonus += zbonusdelta;
		}
	} while (timespec_subtract_ns (&now, &start) <= totalns);
	n->release (ul);
	n->release (ur);
	n->release (ll);
	n->release (lr);
	n->release (hl);
	n->release (vl);

	return true;
}
