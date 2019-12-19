#include <config.hh>

#include <clocale>
#include <cstdio>

#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>

using namespace ncpp;

int main ([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	setlocale (LC_ALL, "");
	NotCurses::default_notcurses_options.inhibit_alternate_screen = true;

	NotCurses &nc = NotCurses::get_instance ();
	if (!nc.init ()) {
		return EXIT_FAILURE;
	}

	Plane *stdplane = nc.get_stdplane ();

	int dimx, dimy;
	stdplane->get_dim (&dimy, &dimx);

	Cell c;
	c.set_bg_rgb (0, 0x80, 0);

	if (stdplane->load (c, "🐳") < 0) {
		goto err;
	}

	if (dimy > 5) {
		dimy = 5;
	}

	for (int i = 0 ; i < dimy ; ++i){
		for (int j = 8 ; j < dimx / 2 ; ++j) { // leave some empty spaces
			if (stdplane->putc (i, j * 2, c) < 0){
				goto err;
			}
		}
	}

	stdplane->putc (dimy, dimx - 3, c);
	stdplane->putc (dimy, dimx - 1, c);
	stdplane->putc (dimy + 1, dimx - 2, c);
	stdplane->putc (dimy + 1, dimx - 4, c);
	stdplane->release (c);

	// put these on the right side of the wide glyphs
	for (int i = 0; i < dimy / 2; ++i) {
		for (int j = 5; j < dimx / 2; j += 2) {
			if (stdplane->putc (i, j, (j % 10) + '0') < 0) {
				goto err;
			}
		}
	}

	// put these on the left side of the wide glyphs
	for (int i = dimy / 2; i < dimy; ++i) {
		for (int j = 4; j < dimx / 2; j += 2) {
			if (stdplane->putc (i, j, (j % 10) + '0') < 0){
				goto err;
			}
		}
	}

	if (nc.render ()) {
		goto err;
	}
	printf("\n");

  err:
	return EXIT_FAILURE;
}
