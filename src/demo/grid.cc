#include <config.hh>

#include <unistd.h>
#include "demo.hh"

#include <ncpp/NotCurses.hh>

using namespace ncpp;

// clip and set
static bool
ccell_set_fg_rgb (Cell &c, int r, int g, int b)
{
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	return c.set_fg_rgb (r, g, b);
}

static bool
ccell_set_bg_rgb (Cell &c, int r, int g, int b)
{
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	return c.set_bg_rgb (r, g, b);
}

static void
release_cells (Plane *n,
			   Cell &ul, Cell &uc, Cell &ur,
			   Cell &cl, Cell &cc, Cell &cr,
			   Cell &ll, Cell &lc, Cell &lr)
{
	n->release (ul);
	n->release (uc);
	n->release (ur);
	n->release (cl);
	n->release (cc);
	n->release (cr);
	n->release (ll);
	n->release (lc);
	n->release (lr);
}

static int
prep_cells2 (Plane *n,
			 Cell &ul, Cell &uc, Cell &ur,
			 Cell &cl, Cell &cc, Cell &cr,
			 Cell &ll, Cell &lc, Cell &lr)
{
	ul.init ();
	uc.init ();
	cl.init ();
	cr.init ();
	ll.init ();
	lc.init ();
	lr.init ();
	ur.init ();
	cc.init ();

	int ret = 0;
	ret |= n->load (ul, "╔");
	ret |= n->load (uc, "╦");
	ret |= n->load (ur, "╗");
	ret |= n->load (cl, "╠");
	ret |= n->load (cc, "╬");
	ret |= n->load (cr, "╣");
	ret |= n->load (ll, "╚");
	ret |= n->load (lc, "╩");
	ret |= n->load (lr, "╝");

	return ret;
}

static int
prep_cells (Plane *n,
			Cell &ul, Cell &uc, Cell &ur,
			Cell &cl, Cell &cc, Cell &cr,
			Cell &ll, Cell &lc, Cell &lr)
{
	ul.init ();
	uc.init ();
	cl.init ();
	cr.init ();
	ll.init ();
	lc.init ();
	lr.init ();
	ur.init ();
	cc.init ();

	int ret = 0;
	ret |= n->load (ul, "┍");
	ret |= n->load (uc, "┯");
	ret |= n->load (ur, "┑");
	ret |= n->load (cl, "┝");
	ret |= n->load (cc, "┿");
	ret |= n->load (cr, "┥");
	ret |= n->load (ll, "┕");
	ret |= n->load (lc, "┷");
	ret |= n->load (lr, "┙");

	return ret;
}

static int
bgnext (Cell &c, int* r, int* g, int* b)
{
	int ret = ccell_set_bg_rgb (c, *r, *g, *b);
	if (*g % 2) {
		if (--*b <= 0) {
			if (++*g >= 256) {
				*g = 255;
			}
			*b = 0;
		}
	} else {
		if (++*b >= 256) {
			if (++*g >= 256) {
				*g = 255;
			}
			*b = 255;
		}
	}

	return ret;
}

static bool
gridswitch_demo (NotCurses &nc, Plane *n)
{
	n->erase ();

	int ret = 0;
	int maxx, maxy;
	Cell ul, ll, cl, cr, lc, lr, ur, uc, cc;
	prep_cells (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);

	for (int i = 0 ; i < 256 ; ++i) {
		nc.get_term_dim (&maxy, &maxx);
		int rs = 256 / maxx;
		int gs = 256 / (maxx + maxy);
		int bs = 256 / maxy;
		int x = 0;
		int y = 0;
		int bgr = i;
		int bgg = 0x80;
		int bgb = i;

		// top line
		ret |= ccell_set_fg_rgb (ul, 255 - rs * y, 255 - gs * (x + y), 255 - bs * y);
		ret |= bgnext (ul, &bgr, &bgg, &bgb);
		if (n->putc (y, x, ul) <= 0) {
			return false;
		}

		for (x = 1 ; x < maxx - 1 ; ++x) {
			ret |= ccell_set_fg_rgb (uc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
			ret |= bgnext (uc, &bgr, &bgg, &bgb);
			if(n->putc (uc) <= 0) {
				return false;
			}
		}
		ret |= ccell_set_fg_rgb (ur, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
		ret |= bgnext (ur, &bgr, &bgg, &bgb);
		if (n->putc (ur) < 0) {
			return false;
		}

		// center
		for (y = 1 ; y < maxy - 1 ; ++y) {
			x = 0;
			ret |= ccell_set_fg_rgb (cl, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
			ret |= bgnext (cl, &bgr, &bgg, &bgb);
			n->putc (y, x, cl);

			for (x = 1 ; x < maxx - 1 ; ++x) {
				ret |= ccell_set_fg_rgb (cc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
				ret |= bgnext (cc, &bgr, &bgg, &bgb);
				n->putc (cc);
			}

			ret |= ccell_set_fg_rgb (cr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
			ret |= bgnext (cr, &bgr, &bgg, &bgb);
			n->putc (cr);
		}

		// bottom line
		x = 0;
		ret |= ccell_set_fg_rgb (ll, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
		ret |= bgnext (ll, &bgr, &bgg, &bgb);
		n->putc (y, x, ll);

		for (x = 1 ; x < maxx - 1 ; ++x) {
			ret |= ccell_set_fg_rgb (lc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
			ret |= bgnext (lc, &bgr, &bgg, &bgb);
			n->putc (lc);
		}

		ret |= ccell_set_fg_rgb (lr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * x);
		ret |= bgnext (lr, &bgr, &bgg, &bgb);
		n->putc (lr);

		// render!
		demo_render (nc);
	}

	release_cells (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);
	return ret;
}

static bool
gridinv_demo (NotCurses &nc, Plane *n)
{
	n->erase ();

	Cell ul, ll, cl, cr, lc, lr, ur, uc, cc;
	prep_cells2 (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);
	for (int i = 0 ; i < 256 ; ++i) {
		int maxx, maxy;
		nc.get_term_dim (&maxy, &maxx);

		int rs = 255 / maxx;
		int gs = 255 / (maxx + maxy);
		int bs = 255 / maxy;
		int x = 0;
		int y = 0;

		// top line
		ccell_set_fg_rgb (ul, i, x * rs, y * bs);
		ccell_set_bg_rgb (ul, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		n->putc (y, x, ul);

		for (x = 1 ; x < maxx - 1 ; ++x) {
			ccell_set_fg_rgb (uc, i, x * rs, y * bs);
			ccell_set_bg_rgb (uc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			n->putc (uc);
		}

		ccell_set_fg_rgb (ur, i, x * rs, y * bs);
		ccell_set_bg_rgb (ur, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		n->putc (ur);

		// center
		for (y = 1 ; y < maxy - 1 ; ++y) {
			x = 0;

			ccell_set_fg_rgb (cl, i, x * rs, y * bs);
			ccell_set_bg_rgb (cl, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			n->putc (y, x, cl);

			for (x = 1 ; x < maxx - 1 ; ++x) {
				ccell_set_fg_rgb (cc, i, x * rs, y * bs);
				ccell_set_bg_rgb (cc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
				n->putc (cc);
			}

			ccell_set_fg_rgb (cr, i, x * rs, y * bs);
			ccell_set_bg_rgb (cr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			n->putc (cr);
		}

		// bottom line
		x = 0;
		ccell_set_fg_rgb (ll, i, x * rs, y * bs);
		ccell_set_bg_rgb (ll, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		n->putc (y, x, ll);
		for (x = 1 ; x < maxx - 1 ; ++x) {
			ccell_set_fg_rgb (lc, i, x * rs, y * bs);
			ccell_set_bg_rgb (lc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			n->putc (lc);
		}

		ccell_set_fg_rgb (lr, i, x * rs, y * bs);
		ccell_set_bg_rgb (lr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		n->putc (lr);

		demo_render (nc);
	}
	release_cells (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);
	return gridswitch_demo (nc, n);
}

// red across, blue down, green from UL to LR
bool grid_demo (NotCurses &nc)
{

	int y, x;
	Plane *n = nc.get_stdplane ();
	n->erase ();

	Cell ul, uc, ur;
	Cell ll, lc, lr;
	Cell cl, cc, cr;
	prep_cells (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);

	bool failure = false;
	for (int i = 0 ; i < 256 ; ++i) {
		int maxx, maxy;

		nc.get_term_dim (&maxy, &maxx);
		int rs = 255 / maxx;
		int gs = 255 / (maxx + maxy);
		int bs = 255 / maxy;

		// top line
		x = 0;
		y = 0;
		failure |= !ccell_set_bg_rgb (ul, i, x * rs, y * bs);
		failure |= !ccell_set_fg_rgb (ul, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		if (n->putc (0, 0, &ul) <= 0) {
			return false;
		}

		for (x = 1 ; x < maxx - 1 ; ++x) {
			failure |= !ccell_set_bg_rgb (uc,  i, x * rs, y * bs);
			failure |= !ccell_set_fg_rgb (uc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			if (n->putc (uc) <= 0) {
				return false;
			}
		}

		// center
		for (y = 1 ; y < maxy - 1 ; ++y) {
			x = 0;
			failure |= !ccell_set_bg_rgb (cl, i, x * rs, y * bs);
			failure |= !ccell_set_fg_rgb (cl, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			if (n->putc (y, x, &cl) <= 0) {
				return false;
			}

			for (x = 1 ; x < maxx - 1 ; ++x) {
				failure |= !ccell_set_bg_rgb (cc, i, x * rs, y * bs);
				failure |= !ccell_set_fg_rgb (cc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
				if (n->putc (cc) <= 0) {
					return false;
				}
			}
			failure |= !ccell_set_bg_rgb (cr, i, x * rs, y * bs);
			failure |= !ccell_set_fg_rgb (cr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			if (n->putc (cr) <= 0) {
				return false;
			}
		}

		// bottom line
		x = 0;
		failure |= !ccell_set_bg_rgb (ll, i, x * rs, y * bs);
		failure |= !ccell_set_fg_rgb (ll, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		if (n->putc (y, x, &ll) <= 0) {
			return false;
		}

		for (x = 1 ; x < maxx - 1 ; ++x) {
			failure |= !ccell_set_bg_rgb (lc, i, x * rs, y * bs);
			failure |= !ccell_set_fg_rgb (lc, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
			if (n->putc (lc) <= 0) {
				return false;
			}
		}
		failure |= !ccell_set_bg_rgb (lr, i, x * rs, y * bs);
		failure |= !ccell_set_fg_rgb (lr, 255 - rs * x, 255 - gs * (x + y), 255 - bs * y);
		if (n->putc (lr) <= 0) {
			return false;
		}

		if (failure || !demo_render (nc)) {
			return false;
		}
	}

	release_cells (n, ul, uc, ur, cl, cc, cr, ll, lc, lr);
	return gridinv_demo (nc, n);
}
