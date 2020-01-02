#include <config.hh>

#include <memory>

#include "demo.hh"

using namespace ncpp;

static const char* leg[] = {
	"                              88              88            88           88                          88             88               88                        ",
	"                              \"\"              88            88           88                          88             \"\"               \"\"                 ,d     ",
	"                                              88            88           88                          88                                                 88     ",
	"  ,adPPYYba,     8b,dPPYba,   88   ,adPPYba,  88   ,d8      88,dPPYba,   88  ,adPPYYba,   ,adPPYba,  88   ,d8       88   ,adPPYba,   88  8b,dPPYba,  MM88MMM   ",
	"  \"\"     `Y8     88P'   `\"8a  88  a8\"     \"\"  88 ,a8\"       88P'    \"8a  88  \"\"     `Y8  a8\"     \"\"  88 ,a8\"        88  a8\"     \"8a  88  88P'   `\"8a   88      ",
	"  ,adPPPPP88     88       88  88  8b          8888[         88       d8  88  ,adPPPPP88  8b          8888[          88  8b       d8  88  88       88   88      ",
	"  88,    ,88     88       88  88  \"8a,   ,aa  88`\"Yba,      88b,   ,a8\"  88  88,    ,88  \"8a,   ,aa  88`\"Yba,       88  \"8a,   ,a8\"  88  88       88   88,     ",
	"  `\"8bbdP\"Y8     88       88  88   `\"Ybbd8\"'  88   `Y8a     8Y\"Ybbd8\"'   88  `\"8bbdP\"Y8   `\"Ybbd8\"'  88   `Y8a      88   `\"YbbdP\"'   88  88       88   \"Y888   ",
	"                                                                                                                   ,88                                         ",
	"                                                                                                                 888P                                          ",
};

// FIXME: find a better way to get the instance of NotCurses (or not?)
static int
perframecb ([[maybe_unused]] struct notcurses *_nc, [[maybe_unused]] ncvisual* ncv, [[maybe_unused]] void *vnewplane)
{
	NotCurses &nc = NotCurses::get_instance ();
	static int startr = 0x80;
	static int startg = 0xff;
	static int startb = 0x80;
	static int frameno = 0;
	int dimx, dimy, y;
	std::shared_ptr<Plane> n;

	if (vnewplane != nullptr) {
		n = *static_cast<std::shared_ptr<Plane>*>(vnewplane);
	}

	if (!n) {
		Plane* nstd = nc.get_stdplane ();
		nstd->get_dim (&dimy, &dimx);
		//y = dimy - sizeof(leg) / sizeof(*leg) - 1;
		y = 0;
		n = std::make_shared<Plane> (sizeof(leg) / sizeof(*leg), dimx, y, 0);
	}

	n->get_dim (&dimy, &dimx);
	Cell c(' ');
	c.set_fg_alpha (Cell::AlphaTransparent);
	c.set_bg_alpha (Cell::AlphaTransparent);
	n->set_base (c);
	n->set_fg_alpha (Cell::AlphaBlend);
	n->set_bg_alpha (Cell::AlphaBlend);

	// fg/bg rgbs are set within loop
	int x = dimx - frameno;
	int r = startr;
	int g = startg;
	int b = startb;
	const size_t llen = strlen (leg[0]);
	do{
		if (x + (int)llen <= 0) {
			x += llen;
		} else {
			int len = dimx - x;
			if (x < 0) {
				len = llen + x;
			}

			if (len > (int)llen) {
				len = llen;
			}

			if (len > dimx) {
				len = dimx;
			}

			int stroff = 0;
			if (x < 0) {
				stroff = -x;
				x = 0;
			}

			for (size_t l = 0 ; l < sizeof(leg) / sizeof(*leg) ; ++l, ++y) {
				if (!n->set_fg_rgb (r - 0xc * l, g - 0xc * l, b - 0xc * l)) {
					return -1;
				}

				if (!n->set_bg_rgb ((l + 1) * 0x2, 0x20, (l + 1) * 0x2)) {
					return -1;
				}

				if (n->printf (l, x, "%*.*s", len, len, leg[l] + stroff) != (int)len) {
					return -1;
				}
			}
			x += len;
		}
		int t = r;
		r = g;
		g = b;
		b = t;
	} while (x < dimx);

	++frameno;
	demo_render(nc);
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

	ncv->stream (&averr, perframecb, &n);

	return true;
}
