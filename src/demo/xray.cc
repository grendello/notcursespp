#include <config.hh>

#include <memory>

#include "demo.hh"

using namespace ncpp;

static const char* leg[] = {
"                               88              88            88           88                          88             88               88                        ",
"                               \"\"              88            88           88                          88             \"\"               \"\"                 ,d     ",
"                                               88            88           88                          88                                                 88     ",
"   ,adPPYYba,     8b,dPPYba,   88   ,adPPYba,  88   ,d8      88,dPPYba,   88  ,adPPYYba,   ,adPPYba,  88   ,d8       88   ,adPPYba,   88  8b,dPPYba,  MM88MMM   ",
"   \"\"     `Y8     88P'   `\"8a  88  a8\"     \"\"  88 ,a8\"       88P'    \"8a  88  \"\"     `Y8  a8\"     \"\"  88 ,a8\"        88  a8\"     \"8a  88  88P'   `\"8a   88      ",
"   ,adPPPPP88     88       88  88  8b          8888[         88       d8  88  ,adPPPPP88  8b          8888[          88  8b       d8  88  88       88   88      ",
"   88,    ,88     88       88  88  \"8a,   ,aa  88`\"Yba,      88b,   ,a8\"  88  88,    ,88  \"8a,   ,aa  88`\"Yba,       88  \"8a,   ,a8\"  88  88       88   88,     ",
"   `\"8bbdP\"Y8     88       88  88   `\"Ybbd8\"'  88   `Y8a     8Y\"Ybbd8\"'   88  `\"8bbdP\"Y8   `\"Ybbd8\"'  88   `Y8a      88   `\"YbbdP\"'   88  88       88   \"Y888   ",
"                                                                                                                    ,88                                         ",
"                                                                                                                  888P                                          ",
};

// FIXME: find a better way to get the instance of NotCurses (or not?)
static int
perframecb ([[maybe_unused]] struct notcurses *_nc, [[maybe_unused]] ncvisual* ncv)
{
	NotCurses &nc = NotCurses::get_instance ();
	static std::shared_ptr<Plane> n;
	static int startr = 0x80;
	static int startg = 0xff;
	static int startb = 0x80;
	static int frameno = 0;

	int dimx, dimy, y;
	if (n == nullptr) {
		Plane *nstd = nc.get_stdplane ();
		nstd->get_dim (&dimy, &dimx);
		y = dimy - sizeof(leg) / sizeof(*leg);
		// FIXME how will this plane be destroyed?
		n = std::make_shared<Plane> (sizeof(leg) / sizeof(*leg), dimx, y, 0);
	    if (n == nullptr) {
			return false;
		}
	}
	n->get_dim (&dimy, &dimx);

	y = 0;
	Cell c(' ');
	c.set_fg_alpha (Cell::AlphaTransparent);
	c.set_bg_alpha (Cell::AlphaTransparent);
	n->set_default (c);
	n->set_fg_alpha (Cell::AlphaBlend);
	n->set_bg_alpha (Cell::AlphaBlend);
	// fg/bg rgbs are set within loop
	int x = dimx - frameno;
	for (size_t l = 0 ; l < sizeof(leg) / sizeof(*leg) ; ++l, ++y) {
		int r = startr;
		int g = startg - (l * 0x8);
		int b = startb;

		n->set_bg_rgb (l * 0x4, 0x20, l * 0x4);
		int xoff = x;
		while (xoff + (int)strlen (leg[l]) <= 0){
			xoff += strlen (leg[l]);
		}

		do {
			n->set_fg_rgb (r, g, b);
			int len = dimx - xoff;

			if (xoff < 0) {
				len = strlen (leg[l]) + xoff;
			} else if (xoff == 0) {
				int t = startr;
				startr = startg;
				startg = startb;
				startb = t;
			}

			if (len > (int)strlen (leg[l])){
				len = strlen (leg[l]);
			}

			if (len > dimx) {
				len = dimx;
			}

			int stroff = 0;
			if (xoff < 0) {
				stroff = -xoff;
				xoff = 0;
			}

			n->printf (y, xoff, "%*.*s", len, len, leg[l] + stroff);
			xoff += len;
			int t = r;
			r = g;
			g = b;
			b = t;
		} while (xoff < dimx);
	}
	++frameno;
	nc.render ();
	// FIXME we'll need some delay here
	return 0;
}

bool xray_demo (NotCurses &nc)
{
	int dimx, dimy;
	Plane* nstd = nc.get_stdplane ();
	nstd->get_dim (&dimy, &dimx);
	std::shared_ptr<Plane> n = std::make_shared<Plane> (dimy, dimx, 0, 0);
	if (n == nullptr || !*n) {
		return false;
	}
	char* path = find_data ("notcursesI.avi");
	int averr;
	std::unique_ptr<Visual> ncv (n->visual_open (path, &averr));
	if (ncv == nullptr || !*ncv) {
		return false;
	}

	if (ncv->decode (&averr) == nullptr) {
		return false;
	}

	ncv->stream (&averr, perframecb);

	return true;
}
