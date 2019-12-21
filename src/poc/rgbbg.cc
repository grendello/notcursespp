#include <config.hh>

#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <iostream>

#include <ncpp/NotCurses.hh>

using namespace ncpp;

int main (void)
{
	if (!setlocale (LC_ALL, "")) {
		std::cerr << "Couldn't set locale" << std::endl;
		return EXIT_FAILURE;
	}

	NotCurses::default_notcurses_options.inhibit_alternate_screen = true;
	NotCurses &nc = NotCurses::get_instance ();
	nc.init ();

	if (!nc) {
		return EXIT_FAILURE;
	}

	int y, x, dimy, dimx;
	Plane* n = nc.get_stdplane ();
	n->get_dim (&dimy, &dimx);

	int r , g, b;
	r = 0;
	g = 0x80;
	b = 0;
	n->set_fg_rgb (0x80, 0x80, 0x80);

	for (y = 0 ; y < dimy ; ++y) {
		for (x = 0 ; x < dimx ; ++x) {
			n->set_bg_rgb (r, g, b);
			n->putc ('x');
			if (g % 2) {
				if (b-- == 0) {
					++g;
					b = 0;
				}
			} else {
				if (b++ >= 256) {
					++g;
					b = 256;
				}
			}
		}
	}

	if (!nc.render ()) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
