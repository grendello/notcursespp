#include <config.hh>

#include <unistd.h>
#include <memory>
#include <ncpp/NotCurses.hh>

#include "demo.hh"

using namespace ncpp;

static void
grow_rgb (uint32_t* rgb)
{
	// FIXME: use proper channels API wrapper
	int r = channel_r (*rgb);
	int g = channel_g (*rgb);
	int b = channel_b (*rgb);
	int delta = (*rgb & 0x80000000ul) ? -1 : 1;
	if (b == r) {
		b += delta;
	} else if (g == r) {
		g += delta;
	} else {
		r += delta;
	}

	if (b == 256 || r == 256 || g == 256) {
		b = r = g = 255;
		*rgb |= 0x80000000ul;
	} else if (b == -1 || r == -1 || g == -1) {
		b = r = g = 0;
		*rgb &= ~0x80000000ul;
	}
	*rgb = (*rgb & 0xff000000ul) | (r * 65536 + g * 256 + b);
}

static std::shared_ptr<Plane>
legend (NotCurses &nc, const char* msg)
{
	int dimx, dimy;
	nc.get_term_dim (&dimy, &dimx);

	// FIXME replace with notcurses_newplane_aligned()
	auto n = std::make_shared<Plane> (3, strlen(msg) + 2, dimy - 4, (dimx - ((strlen(msg) + 2))) / 2);

	Cell c;
	c.set_fg_rgb (0, 0, 0); // darken surrounding characters by half
	c.set_fg_alpha (Cell::AlphaBlend);
	c.set_bg_alpha (Cell::AlphaTransparent); // don't touch background
	n->set_base (c);
	n->set_fg (0xd78700);
	n->set_bg (0);
	if (n->putstr (1, 1, msg) < 0) {
		return nullptr;
	}

	return n;
}

static bool
slideitslideit (NotCurses &nc, std::shared_ptr<Plane> n, uint64_t deadline, int *direction)
{
	int dimy, dimx;
	int yoff, xoff;
	int ny, nx;

	nc.get_term_dim (&dimy, &dimx);
	n->get_dim (&ny, &nx);
	n->get_yx (&yoff, &xoff);

	struct timespec iterdelay = { /*.tv_sec =*/ 0, /*.tv_nsec =*/ 50000000, };
	struct timespec cur;

	do{
		if (!demo_render (nc)) {
			return false;
		}

		switch (*direction) {
			case 0: --yoff; --xoff; break;
			case 1: --yoff; ++xoff; break;
			case 2: ++yoff; ++xoff; break;
			case 3: ++yoff; --xoff; break;
		}

		if (xoff == 0) {
			++xoff;
			if (*direction == 0) {
				*direction = 1;
			} else if (*direction == 3) {
				*direction = 2;
			}
		} else if (xoff == dimx - nx) {
			--xoff;
			if (*direction == 1) {
				*direction = 0;
			} else if (*direction == 2) {
				*direction = 3;
			}
		}

		if (yoff == 0) {
			++yoff;
			if (*direction == 0) {
				*direction = 3;
			} else if (*direction == 1) {
				*direction = 2;
			}
		} else if (yoff == dimy - ny) {
			--yoff;
			if (*direction == 2) {
				*direction = 1;
			} else if (*direction == 3) {
				*direction = 0;
			}
		}
		n->move (yoff, xoff);
		nanosleep (&iterdelay, nullptr);
		clock_gettime (CLOCK_MONOTONIC, &cur);
	} while (timespec_to_ns (&cur) < deadline);

	return true;
}

// run panels atop the display in an exploration of transparency
static bool
slidepanel (NotCurses &nc)
{
	const int DELAYSCALE = 2;
	int dimy, dimx;

	nc.get_term_dim (&dimy, &dimx);

	int ny = dimy / 4;
	int nx = dimx / 3;
	int yoff = random () % (dimy - ny - 2) + 1; // don't start atop a border
	int xoff = random () % (dimx - nx - 2) + 1;

	// First we just create a plane with no styling and no glyphs.
	auto n = std::make_shared<Plane> (ny, nx, yoff, xoff);

	// Zero-initialized channels use the default color, opaquely. Since we have
	// no glyph, we should show underlying glyphs in the default colors. The
	// background default might be transparent, at the window level (i.e. a copy
	// of the underlying desktop).
	Cell c (' ');

	struct timespec cur;
	n->set_base (c);
	clock_gettime (CLOCK_MONOTONIC, &cur);

	uint64_t deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	int direction = random () % 4;

	std::shared_ptr<Plane> l = legend (nc, "default background, all opaque, whitespace glyph");
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	n->load (c, '\0');
	n->set_base (c);
	clock_gettime (CLOCK_MONOTONIC, &cur);
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	l = legend (nc, "default background, all opaque, no glyph");
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Next, we set our foreground transparent, allowing characters underneath to
	// be seen in their natural colors. Our background remains opaque+default.
	c.set_fg_alpha (Cell::AlphaTransparent);
	n->set_base (c);

	clock_gettime (CLOCK_MONOTONIC, &cur);
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	l = legend(nc, "default background, fg transparent, no glyph");
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Set the foreground color, setting it to blend. We should get the underlying
	// glyphs in a blended color, with the default background color.
	c.set_fg (0x80c080);
	c.set_fg_alpha (Cell::AlphaBlend);
	n->set_base (c);
	clock_gettime (CLOCK_MONOTONIC, &cur);
	l = legend (nc, "default background, fg blended, no glyph");
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Opaque foreground color. This produces underlying glyphs in the specified,
	// fixed color, with the default background color.
	c.set_fg (0x80c080);
	c.set_fg_alpha (Cell::AlphaOpaque);
	n->set_base (c);
	clock_gettime (CLOCK_MONOTONIC, &cur);
	l = legend (nc, "default background, fg colored opaque, no glyph");
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Now we replace the characters with X's, colored as underneath us.
	// Our background color remains opaque default.
	n->load (c, 'X');
	c.set_fg_default ();
	c.set_fg_alpha (Cell::AlphaTransparent);
	c.set_bg_alpha (Cell::AlphaOpaque);
	n->set_base (c);

	clock_gettime (CLOCK_MONOTONIC, &cur);
	l = legend (nc, "default colors, fg transparent, print glyph");
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Now we replace the characters with X's, but draw the foreground and
	// background color from below us.
	c.set_fg_alpha (Cell::AlphaTransparent);
	c.set_bg_alpha (Cell::AlphaTransparent);
	n->set_base (c);

	clock_gettime (CLOCK_MONOTONIC, &cur);
	l = legend (nc, "all transparent, print glyph");
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	// Finally, we populate the plane for the first time with non-transparent
	// characters. We blend, however, to show the underlying color in our glyphs.
	c.set_fg_alpha (Cell::AlphaBlend);
	c.set_bg_alpha (Cell::AlphaBlend);
	c.set_fg (0x80c080);
	c.set_bg (0x204080);
	n->set_base (c);

	clock_gettime (CLOCK_MONOTONIC, &cur);
	l = legend (nc, "all blended, print glyph");
	deadlinens = timespec_to_ns (&cur) + DELAYSCALE * timespec_to_ns (&demodelay);
	if (!slideitslideit (nc, n, deadlinens, &direction)) {
		return false;
	}
	l.reset ();

	return true;
}

// draws a border along the perimeter, then fills the inside with position
// markers, each a slightly different color. the goal is to make sure we can
// have a great many colors, that they progress reasonably through the space,
// and that we can write to every coordinate.
bool maxcolor_demo (NotCurses &nc)
{
	int maxx, maxy;
	nc.get_term_dim (&maxy, &maxx);

	Plane *n = nc.get_stdplane ();
	n->set_fg_rgb (255, 255, 255);

	uint64_t channels = 0;
	channels_set_fg_rgb(&channels, 0, 128, 128);
	channels_set_bg_rgb(&channels, 90, 0, 90);

	int y = 0, x = 0;
	n->cursor_move (y, x);
	if (!n->rounded_box_sized (0, channels, maxy, maxx, 0)) {
		return false;
	}

	uint32_t rgb = 0;
	for (y = 1 ; y < maxy - 1 ; ++y) {
		x = 1;
		if (!n->cursor_move (y, x)) {
			return false;
		}

		while (x < maxx - 1) {
			n->set_fg_rgb ((rgb & 0xff0000) >> 16u, (rgb & 0xff00) >> 8u, rgb & 0xff);
			n->set_bg_rgb (0, 10, 0);
			n->putc (x % 10 + '0');
			grow_rgb(&rgb);
			++x;
		}
	}

	std::shared_ptr<Plane> l = legend (nc, "what say we explore transparency together?");
	if (!demo_render (nc)) {
		return false;
	}
	l.reset ();

	nanosleep (&demodelay, nullptr);

	return slidepanel (nc);
}
