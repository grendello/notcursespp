#include <config.hh>

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
	static int startr = 0x80;
	static int startg = 0xff;
	static int startb = 0x80;
	static int frameno = 0;
	int dimx, dimy;
	Plane* n = nc.get_stdplane ();
	n->get_dim (&dimy, &dimx);
	n->putc (0, 0, 'a');

	int y = dimy - sizeof(leg) / sizeof(*leg) - 3;
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
	Plane* n = nc.get_stdplane ();
	char* path = find_data ("notcursesI.avi");
	int averr;
	Visual* ncv = n->visual_open (path, &averr);
	if (ncv == nullptr || !*ncv) {
		return false;
	}

	if (ncv->decode (&averr) == nullptr) {
		return false;
	}

	ncv->stream (&averr, perframecb);
	delete ncv;

	return true;
}
