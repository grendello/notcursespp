#include <config.hh>

#include "demo.hh"

using namespace ncpp;

// we provide a heads-up display throughout the demo, detailing the demos we're
// about to run, running, and just runned. the user can move this HUD with
// their mouse. it should always be on the top of the z-stack.
Plane* hud = nullptr;

// while the HUD is grabbed by the mouse, these are set to the position where
// the grab started. they are reset once the HUD is released.
static int hud_grab_x = -1;
static int hud_grab_y = -1;
// position of the HUD *when grab started*
static int hud_pos_x;
static int hud_pos_y;

constexpr int HUD_ROWS = 3;
constexpr int HUD_COLS = 30;

typedef struct elem {
	char* name;
	uint64_t startns;
	uint64_t totalns;
	struct elem* next;
} elem;

static struct elem* elems;
static struct elem* running;
// which line we're writing the next entry to. once this becomes -1, we stop decrementing
// it, and throw away the oldest entry each time.
static int writeline = HUD_ROWS - 1;

// FIXME: do something about the callbacks...
int demo_fader ([[maybe_unused]] struct notcurses *nc, [[maybe_unused]] struct ncplane *ncp)
{
	return demo_render (NotCurses::get_instance ());
}

static bool
hud_standard_bg (Plane* n)
{
	Cell c(' ');
	c.set_bg_rgb (0xc0, 0xf0, 0xc0);
	c.set_bg_alpha (Cell::AlphaBlend);
	n->set_base (c);
	n->release (c);
	return true;
}

static bool
hud_grabbed_bg (Plane* n)
{
	Cell c(' ');
	c.set_bg_rgb (0x40, 0x90, 0x40);
	n->set_base (c);
	n->release (c);

	return true;
}

Plane* hud_create (NotCurses &nc)
{
	int dimx, dimy;
	nc.get_term_dim (&dimy, &dimx);

	int xoffset = (dimx - HUD_COLS) / 2;
	//int yoffset = (dimy - HUD_ROWS);
	int yoffset = 0;
	auto n = new Plane (HUD_ROWS, HUD_COLS, yoffset, xoffset);
	hud_standard_bg (n);

	uint64_t channels = 0;
	channels_set_fg (&channels, 0xffffff);
	channels_set_bg (&channels, 0xffffff);
	n->set_bg (0x409040);
	if (n->putc (0, HUD_COLS - 1, "\u2612", 0, channels, nullptr) < 0) {
		delete n;
		return nullptr;
	}

	return (hud = n);
}

bool hud_destroy (void)
{
	if (hud != nullptr) {
		delete hud;
		hud = nullptr;
	}

	return true;
}

// mouse has been pressed on the hud. the caller is responsible for rerendering.
bool hud_grab (int y, int x)
{
	bool ret;
	if (hud == nullptr) {
		return false;
	}

	// are we in the middle of a grab?
	if (hud_grab_x >= 0 && hud_grab_y >= 0) {
		int delty = y - hud_grab_y;
		int deltx = x - hud_grab_x;
		ret = hud->move (hud_pos_y + delty, hud_pos_x + deltx);
	} else {
		// new grab. stash point of original grab, and location of HUD at original
		// grab. any delta while grabbed (relative to the original grab point)
		// will see the HUD moved by delta (relative to the original HUD location).
		hud_grab_x = x;
		hud_grab_y = y;
		hud->get_yx (&hud_pos_y, &hud_pos_x);
		if (x == hud_pos_x + HUD_COLS - 1 && y == hud_pos_y) {
			return hud_destroy ();
		}
		ret = hud_grabbed_bg (hud);
	}

	return ret;
}

bool hud_release(void)
{
	if (hud == nullptr) {
		return false;
	}

	hud_grab_x = -1;
	hud_grab_y = -1;
	return hud_standard_bg (hud);
}

// currently running demo is always at y = HUD_ROWS-1
bool hud_completion_notify (const demoresult* result)
{
	if (running) {
		running->totalns = result->timens;
	}
	return true;
}

// inform the HUD of an upcoming demo
bool hud_schedule (const char* demoname)
{
	if (hud == nullptr) {
		return false;
	}

	Cell c;
	hud->get_base (c);
	hud->set_bg (c.get_bg ());
	hud->set_bg_alpha (Cell::AlphaBlend);
	hud->set_fg (0);

	elem* cure;
	elem** hook = &elems;
	int line = writeline;
	// once we pass through this conditional:
	//  * cure is ready to write to, and print at y = HUD_ROWS - 1
	//  * hooks is ready to enqueue cure to
	//  * reused entries have been printed, if any exist
	if (line == -1) {
		cure = elems;
		elems = cure->next;
		line = 0;
		free (cure->name);
	} else {
		--writeline;
		cure = new elem;
	}

	elem* e = elems;
	int nslen = 14;
	int plen = HUD_COLS - 4 - nslen;
	while (e) {
		hook = &e->next;
		if (!hud->printf (line, 0, "%*luns %*.*s", nslen, e->totalns, plen, plen, e->name)) {
			return false;
		}
		++line;
		e = e->next;
	}

	*hook = cure;
	cure->name = strdup(demoname);
	cure->next = nullptr;
	cure->totalns = 0;

	struct timespec cur;
	clock_gettime (CLOCK_MONOTONIC, &cur);
	cure->startns = timespec_to_ns (&cur);
	running = cure;
	if (!hud->printf (line, 0, "%*luns %-*.*s", nslen, cure->totalns, plen, plen, cure->name)) {
		return false;
	}

	return true;
}

bool demo_render (NotCurses &nc)
{
	if (hud != nullptr) {
		int nslen = 14;
		int plen = HUD_COLS - 4 - nslen;
		hud->move_top ();

		struct timespec ts;
		clock_gettime (CLOCK_MONOTONIC, &ts);
		if (!hud->printf (HUD_ROWS - 1, 0, "%*luns %-*.*s", nslen,
						  timespec_to_ns(&ts) - running->startns,
						  plen, plen, running->name)) {
			return false;
		}
	}

	return nc.render ();
}
