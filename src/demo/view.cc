#include <config.hh>

#include <memory>

#include <ncpp/NotCurses.hh>
#include "demo.hh"

using namespace ncpp;

static int
watch_for_keystroke ([[maybe_unused]] struct notcurses* nc, [[maybe_unused]] struct ncvisual* ncv, [[maybe_unused]] void* curry)
{
	wchar_t w;
	// we don't want a keypress, but should handle NCKEY_RESIZE
	if ((w = demo_getc_nblock (nullptr)) != (wchar_t)-1) {
		if (w == NCKEY_RESIZE) {
			// FIXME resize that sumbitch
		} else if (w) {
			return 1;
		}
	}

	return demo_render (NotCurses::get_instance ()) ? 0 : 1;
}

static bool
view_video_demo (NotCurses &nc)
{
	std::unique_ptr<Plane> ncp (nc.get_stdplane ());
	int dimy, dimx;
	ncp->get_dim (&dimy, &dimx);

	int averr;
	char* fm6 = find_data ("fm6.mkv");
	std::unique_ptr<Visual> ncv (ncp->visual_open (fm6, &averr));
	delete[] fm6;

	if (!ncv) {
		return false;
	}

	if (ncv->stream (&averr, watch_for_keystroke) < 0) {
		return false;
	}

	return true;
}

static std::shared_ptr<Plane>
legend (int dimy, int dimx)
{
	auto n = std::make_shared<Plane> (4, 25, dimy * 7 / 8, (dimx - 25) / 2);
	n->set_bg_alpha (Cell::AlphaTransparent);

	uint64_t channels = 0;
	channels_set_bg_alpha (&channels, Cell::AlphaTransparent);

	Cell c(' ', 0, channels);
	n->set_base (c);
	n->styles_set (CellStyle::Bold);
	n->set_fg_rgb (0xff, 0xff, 0xff);
	if (n->putstr (0, NCAlign::Center, "target launch") <= 0) {
		return nullptr;
	}

	n->set_fg_rgb (0, 0, 0);
	n->styles_off (CellStyle::Bold);

	if (n->putstr (1, NCAlign::Center, "2003-12-11 FM-6") <= 0) {
		return nullptr;
	}

	if (n->putstr (2, NCAlign::Center, "RIM-161 SM-3 v. Aries TTV") <= 0) {
		return nullptr;
	}

	n->set_fg_rgb (0x80, 0xc0, 0x80);
	if (n->putstr (3, NCAlign::Center, "exo-atmospheric intercept") <= 0) {
		return nullptr;
	}

	return n;
}

bool view_demo (NotCurses &nc)
{
	std::unique_ptr<Plane> ncp (nc.get_stdplane ());
	int dimy, dimx;
	ncp->get_dim (&dimy, &dimx);

	int averr = 0;
	char* pic = find_data ("PurpleDrank.jpg");
	ncp->erase ();

	std::unique_ptr<Visual> ncv (ncp->visual_open (pic, &averr));
	delete[] pic;

	if (!ncv) {
		return false;
	}

	auto dsplane = std::make_unique<Plane> (dimy, dimx, 0, 0);
	if (!dsplane) {
		return false;
	}

	pic = find_data ("dsscaw-purp.png");
	std::unique_ptr<Visual> ncv2 (dsplane->visual_open (pic, &averr));
	delete[] pic;

	if (!ncv2) {
		return false;
	}

	if (ncv->decode (&averr) == nullptr) {
		return false;
	}

	if (ncv2->decode (&averr) == nullptr) {
		return false;
	}

	if (!ncv2->render (0, 0, 0, 0)) {
		return false;
	}

	demo_render (nc);
	dsplane->move_bottom ();
	nanosleep (&demodelay, nullptr);

	if (!ncv->render (0, 0, 0, 0)) {
		return false;
	}

	Cell b;
	b.set_fg_alpha (Cell::AlphaTransparent);
	b.set_bg_alpha (Cell::AlphaTransparent);
	ncv2->get_plane ()->set_base (b);
	ncv.reset ();
	ncv2.reset ();

	demo_render (nc);
	nanosleep (&demodelay, nullptr);

	dsplane->move_top ();
	demo_render (nc);
	dsplane.reset ();
	nanosleep (&demodelay, nullptr);

	std::shared_ptr<Plane> ncpl = legend (dimy, dimx);
	if (!ncpl) {
		return false;
	}

	if (!view_video_demo (nc)) {
		return false;
	}

	return true;
}
