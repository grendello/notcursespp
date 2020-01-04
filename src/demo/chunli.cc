#include <config.hh>

#include <memory>
#include "demo.hh"

#include <ncpp/Visual.hh>

using namespace ncpp;

constexpr int CHUNS = 8; // 8-part sprite

typedef struct chunli {
	char *path;
	std::shared_ptr<Visual> ncv;
	Plane *n;
} chunli;

// test of sprites from files
bool chunli_demo (NotCurses &nc)
{
	Cell b;
	b.set_fg_alpha (Cell::AlphaTransparent);
	b.set_bg_alpha (Cell::AlphaTransparent);

	struct timespec iterdelay;
	timespec_div (&demodelay, 10, &iterdelay);

	int averr, dimy, dimx;
	chunli chuns[CHUNS];
	char file[PATH_MAX];
	for (int i = 0 ; i < CHUNS ; ++i) {
		nc.resize (&dimy, &dimx);
		snprintf (file, sizeof(file), "chunli%d.bmp", i + 1);
		chuns[i].path = find_data (file);
		chuns[i].ncv = std::make_shared<Visual> (chuns[i].path, &averr, 0, 0, NCScale::None);
		if (!chuns[i].ncv) {
			return false;
		}

		if (chuns[i].ncv->decode (&averr) == nullptr) {
			return false;
		}

		if (!chuns[i].ncv->render (0, 0, 0, 0)) {
			return false;
		}

		chuns[i].n = chuns[i].ncv->get_plane ();
		chuns[i].n->set_base (b);

		int thisx, thisy;
		chuns[i].n->get_dim (&thisy, &thisx);
		if (!chuns[i].n->move ((dimy - thisy) / 2, (dimx - thisx) / 2)) {
			return false;
		}

		// xoff += thisx;
		if (!demo_render(nc)) {
			return false;
		}

		nanosleep (&iterdelay, nullptr);
		chuns[i].ncv.reset ();
		delete[] chuns[i].path;
	}

	char* victory = find_data ("chunlivictory.png");
	auto ncv = std::make_unique<Visual> (victory, &averr, 0, 0, NCScale::None);

	if (!ncv) {
		return false;
	}

	if (ncv->decode (&averr) == nullptr) {
		return false;
	}

	Plane *ncp = ncv->get_plane ();
	ncp->set_base (b);

	const int offsets[] = {
		0, 50, 100, 154, 208, 260, 312, 368, 420, 479, 538, 588, 638, 688, 736, 786, 836, 888, 942
	};

	for (size_t i = 0u ; i < sizeof(offsets) / sizeof(*offsets) - 1 ; ++i) {
		nc.resize (&dimy, &dimx);
		ncp->erase ();
		if (!ncv->render (0, offsets[i], 0, offsets[i + 1] - offsets[i] + 1)) {
			return false;
		}

		if (!demo_render (nc)) {
			return false;
		}

		nanosleep (&iterdelay, nullptr);
	}

	return true;
}
