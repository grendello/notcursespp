#include <config.hh>

#include <libavutil/frame.h>

#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>

#include "demo.hh"

using namespace ncpp;

// 0: transparent
// 1: white
// 2: grey
// 3: black
const char eagle1[] =
	"0000003333333000"
	"0000331113112300"
	"0003111111113130"
	"0031111113133330"
	"3311311111333030"
	"2131111111313000"
	"3331111111311300"
	"0311111111331330"
	"0311311111131130"
	"0323111111133130"
	"0323111111133113"
	"0033213111113313"
	"0003213211111333"
	"0000332321321233"
	"0000003312332223"
	"0000000033003330"
	;

// display the level map scaled to fit entirely within the visual area
Visual*
outzoomed_map (NotCurses &nc, const char* map)
{
	int averr;
	Visual *ncv = Visual::open_plane (map, &averr, 0, 0, NCScale::Scale);
	if (ncv == nullptr || !*ncv) {
		return nullptr;
	}

	if (ncv->decode (&averr) == nullptr) {
		return nullptr;
	}

	if (!ncv->render (0, 0, 0, 0)) {
		return nullptr;
	}

	if (!nc.render ()) {
		return nullptr;
	}

	return ncv;
}

Plane*
zoom_map (NotCurses &nc, const char* map)
{
	int averr;
	// determine size that will be represented on screen at once, and how
	// large that section has been rendered in the outzoomed map. take the map
	// and begin opening it on larger and larger planes that fit on the screen
	// less and less. eventually, reach our natural NCSCALE_NONE size and begin
	// scrolling through the map, whooooooooosh.
	Visual* ncv = Visual::open_plane (map, &averr, 0, 0, NCScale::None);
	if (ncv == nullptr || !*ncv) {
		return nullptr;
	}

	struct AVFrame* frame;
	if ((frame = ncv->decode (&averr)) == nullptr) {
		delete ncv;
		return nullptr;
	}
	delete ncv;

	// we start at the lower left corner of the outzoomed map
	int truex, truey; // dimensions of true display
	nc.term_dim_yx (&truey, &truex);

	int vwidth = frame->width;
	int vx = vwidth;
	int vheight = frame->height; // dimensions of unzoomed map
	int vy = vheight / 2;
	int zoomy = truey;
	int zoomx = truex;
	Plane* zncp = nullptr;
	while (zoomy < vy && zoomx < vx) {
		delete zncp;
		zoomy += 2;
		zoomx += 2;
		zncp = new Plane (zoomy, zoomx, 0, 0, nullptr);
		Visual* zncv = zncp->visual_open (map, &averr);
		if (zncv == nullptr || !*zncv) {
			delete ncv;
			delete zncp;
			return nullptr;
		}

		if (zncv->decode (&averr) == nullptr) {
			delete zncv;
			delete zncp;
			return nullptr;
		}

		if (!zncv->render ((zoomy - truey) * 2, 0, 0, ((float)truex / zoomx) * zoomx)) {
			delete zncv;
			delete zncp;
			return nullptr;
		}

		if (!nc.render ()) {
			delete zncv;
			delete zncp;
			return nullptr;
		}

		delete zncv;
	}

	return zncp;
}

static bool
draw_eagle (Plane* n, const char* sprite)
{
	Cell bgc;
	bgc.set_bg_alpha (Cell::AlphaTransparent);
	n->set_default (bgc);
	n->release (bgc);

	size_t s;
	int sbytes;
	uint64_t channels = 0;
	// optimization so we can elide more color changes, see README's "#perf"
	channels_set_bg_rgb (&channels, 0x00, 0x00, 0x00);
	n->cursor_move_yx (0, 0);
	for (s = 0 ; sprite[s] ; ++s) {
		switch (sprite[s]) {
			case '0':
				n->cursor_move_yx ((s + 1) / 16, (s + 1) % 16);
				break;

			case '1':
				channels_set_fg_rgb (&channels, 0xff, 0xff, 0xff);
				break;

			case '2':
				channels_set_fg_rgb (&channels, 0xe3, 0x9d, 0x25);
				break;

			case '3':
				channels_set_fg_rgb (&channels, 0x3a, 0x84, 0x00);
				break;
		}

		if (sprite[s] != '0') {
			if(n->putc ("\u2588", 0, channels, &sbytes) != 1) {
				return false;
			}
		}
	}

	return true;
}

static bool
eagles (NotCurses &nc)
{
	int truex, truey; // dimensions of true display
	nc.term_dim_yx (&truey, &truex);

	struct timespec flapiter;
	timespec_div (&demodelay, truex / 2, &flapiter);

	const int height = 16;
	const int width = 16;
	struct eagle {
		int yoff, xoff;
		Plane* n;
	} e[3];

	for (size_t i = 0 ; i < sizeof(e) / sizeof(*e) ; ++i) {
		e[i].xoff = 0;
		e[i].yoff = random () % ((truey - height) / 2);
		e[i].n = new Plane (height, width, e[i].yoff, e[i].xoff, nullptr);
		if(e[i].n == nullptr || !*e[i].n) {
			return false;
		}

		if (!draw_eagle(e[i].n, eagle1)) {
			return false;
		}
	}

	int eaglesmoved;
	do{
		eaglesmoved = 0;
		nc.render ();

		for (size_t i = 0 ; i < sizeof(e) / sizeof(*e) ; ++i) {
			if (e[i].xoff >= truex) {
				continue;
			}

			e[i].yoff += random () % (2 + i) - 1;
			if (e[i].yoff < 0) {
				e[i].yoff = 0;
			} else if (e[i].yoff + height >= truey) {
				e[i].yoff = truey - height - 1;
			}
			e[i].xoff += random () % ((truex + 80) / 80);
			e[i].n->move (e[i].yoff, e[i].xoff);
			++eaglesmoved;
		}

		nanosleep (&flapiter, NULL);
	} while (eaglesmoved);

	for (size_t i = 0 ; i < sizeof(e) / sizeof(*e) ; ++i) {
		delete e[i].n;
	}

	return true;
}

// motherfucking eagles!
bool eagle_demo (NotCurses &nc)
{
	char* map = find_data ("eagles.png");
	Visual* zo;
	if ((zo = outzoomed_map (nc, map)) == nullptr) {
		delete map;
		return false;
	}

	delete zo;
	Plane* zncp = zoom_map (nc, map);
	if (zncp == nullptr) {
		delete map;
		return false;
	}
	if (!eagles (nc)) {
		delete zncp;
		return false;
	}
	delete zncp;
	delete map;

	return true;
}
