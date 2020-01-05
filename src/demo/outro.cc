#include <config.hh>

#include <memory>
#include <pthread.h>
#include "demo.hh"

using namespace ncpp;

static int y;
static int targy;
static int xstart;
static std::shared_ptr<Plane> on;
static std::shared_ptr<Visual> chncv;

static int
perframe ([[maybe_unused]] struct notcurses* nc, [[maybe_unused]] struct ncvisual* ncv, void* vthree)
{
	int* three = static_cast<int*>(vthree); // move up one every three callbacks
	demo_render (NotCurses::get_instance ());
	if (y < targy) {
		return 0;
	}
	on->move (y, xstart);
	if (--*three == 0) {
		--y;
		*three = 3;
	}

	return 0;
}

static void*
fadethread ([[maybe_unused]] void* vnc)
{
	NotCurses &nc = NotCurses::get_instance ();
	Plane *ncp = nc.get_stdplane ();
	struct timespec fade = { /*.tv_sec =*/ 2, /*.tv_nsec =*/ 0, };

	ncp->fadeout (fade, demo_fader);
	chncv->destroy ();

	int averr;
	char* path = find_data ("samoa.avi");
	std::unique_ptr<Visual> ncv (ncp->visual_open (path, &averr));
	delete[] path;

	if (!ncv) {
		return nullptr;
	}

	int rows, cols;
	nc.get_term_dim (&rows, &cols);

	auto apiap = std::make_unique<Plane> (1, cols, rows - 1, 0);
	apiap->set_fg_rgb (0xc0, 0x40, 0x80);
	apiap->set_bg_rgb (0, 0, 0);
	apiap->putstr (0, NCAlign::Center, "Apia 🡺 Atlanta. Samoa, tula'i ma sisi ia lau fu'a, lou pale lea!");

	int three = 3;
	ncv->stream (&averr, perframe, &three);
	ncv.reset ();

	ncp->erase ();
	fade.tv_sec = 2;
	fade.tv_nsec = 0;
	nanosleep (&fade, nullptr);

	return vnc;
}

static std::shared_ptr<Plane>
outro_message (NotCurses &nc, int* rows, int* cols)
{
	constexpr char str0[] = " ATL, baby! ATL! ";
	constexpr char str1[] = " throw your hands in the air ";
	constexpr char str2[] = " hack on! —dank❤ ";

	int ystart = *rows - 6;
	auto non = std::make_shared<Plane> (nc.get_stdplane (), 5, strlen (str1) + 4, ystart, NCAlign::Center);
	if (!non) {
		return nullptr;
	}

	int xs;
	non->get_yx (nullptr, &xs);

	Cell bgcell(' ');
	channels_set_bg_rgb (&bgcell.get ().channels, 0x58, 0x36, 0x58);
	if (!non->set_base (bgcell)) {
		return nullptr;
	}
	non->get_dim (rows, cols);

	int ybase = 0;
	// bevel the upper corners
	if (!non->set_bg_alpha (Cell::AlphaTransparent)) {
		return nullptr;
	}

	if (non->putc (ybase, 0, ' ') < 0 || non->putc (' ') < 0) {
		return nullptr;
	}

	if (non->putc (ybase, *cols - 2, ' ') < 0 || non->putc (' ') < 0){
		return nullptr;
	}

	// ...and now the lower corners
	if (non->putc (*rows - 1, 0, ' ') < 0 || non->putc (' ') < 0) {
		return nullptr;
	}

	if (non->putc (*rows - 1, *cols - 2, ' ') < 0 || non->putc (' ') < 0) {
		return nullptr;
	}

	if (!non->set_fg_rgb (0, 0, 0)) {
		return nullptr;
	}

	if (!non->set_bg_rgb (0, 180, 180)) {
		return nullptr;
	}

	if (!non->set_bg_alpha (Cell::AlphaOpaque)) { // FIXME use intermediate
		return nullptr;
	}

	non->styles_on (CellStyle::Bold);
	if (non->putstr (++ybase, NCAlign::Center, str0) < 0) {
		return nullptr;
	}

	non->styles_off (CellStyle::Bold);
	if (non->putstr (++ybase, NCAlign::Center, str1) < 0) {
		return nullptr;
	}

	non->styles_on (CellStyle::Italic);
	if (non->putstr (++ybase, NCAlign::Center, str2) < 0) {
		return nullptr;
	}

	non->styles_off (CellStyle::Italic);
	if (!demo_render (nc)) {
		return nullptr;
	}

	non->release (bgcell);
	*rows = ystart;
	*cols = xs;

	return non;
}

bool outro (NotCurses &nc)
{
	std::unique_ptr<Plane> ncp (nc.get_stdplane ());
	if (!ncp) {
		return false;
	}

	int rows, cols;
	ncp->erase ();
	ncp->get_dim (&rows, &cols);

	int averr = 0;
	char *path = find_data ("changes.jpg");
	chncv.reset (ncp->visual_open (path, &averr));
	delete[] path;

	if (!chncv) {
		return false;
	}

	if (chncv->decode (&averr) == nullptr) {
		return false;
	}

	if (!chncv->render (0, 0, 0, 0)) {
		return false;
	}

	xstart = cols;
	int ystart = rows;
	on = outro_message (nc, &ystart, &xstart);
	y = ystart - 1;
	void* ret = nullptr; // thread result
	if (on) {
		on->move_top ();

		pthread_t tid;
		// will fade across 2s
		targy = 3;
		pthread_create (&tid, nullptr, fadethread, nc);
		pthread_join (tid, &ret);
		on->fadeout (demodelay, demo_fader);
	}

	if (ret == nullptr) {
		return false;
	}

	return on ? true : false;
}
