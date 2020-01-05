#include <config.hh>

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <pthread.h>
#include <sys/poll.h>
#include <memory>
#include <iostream>

#include <ncpp/NotCurses.hh>
#include <ncpp/PanelReel.hh>
#include <ncpp/Tablet.hh>
#include "demo.hh"

using namespace ncpp;

constexpr int INITIAL_TABLET_COUNT = 4;

// FIXME ought just be an unordered_map
typedef struct tabletctx {
	~tabletctx ()
	{
		pr.reset ();
		t.reset ();
	}

	pthread_t tid;
	std::shared_ptr<PanelReel> pr;
	std::shared_ptr<Tablet> t;
	int lines;
	unsigned rgb;
	unsigned id;
	struct tabletctx* next;
	pthread_mutex_t lock;
} tabletctx;

static void
kill_tablet (tabletctx** tctx)
{
	tabletctx* t = *tctx;
	if (t != nullptr) {
		if (pthread_cancel (t->tid)) {
			std::cerr << "Warning: error sending pthread_cancel (" << strerror (errno) << ")" << std::endl;
		}

		if (pthread_join (t->tid, nullptr)) {
			std::cerr << "Warning: error joining pthread (" << strerror (errno) << ")" << std::endl;
		}

		t->pr->del (*t->t);
		*tctx = t->next;
		pthread_mutex_destroy (&t->lock);

		delete t;
	}
}

static bool
kill_active_tablet (std::shared_ptr<PanelReel> pr, tabletctx** tctx)
{
	Tablet* focused = pr->get_focused ();
	tabletctx* t;
	while ((t = *tctx)) {
		if (t->t.get () == focused) {
			*tctx = t->next; // pull it out of the list
			t->next = nullptr; // finish splicing it out
			break;
		}
		tctx = &t->next;
	}

	if (t == nullptr) {
		return false; // wasn't present in our list, wacky
	}

	kill_tablet (&t);
	return true;
}

// We need write in reverse order (since only the bottom will be seen, if we're
// partially off-screen), but also leave unused space at the end (since
// wresize() only keeps the top and left on a shrink).
static int
tabletup (Plane *w, int begx, int begy, int maxx, int maxy, tabletctx* tctx, int rgb)
{
	char cchbuf[2];
	Cell c;
	int y, idx;

	idx = tctx->lines;
	if (maxy - begy > tctx->lines) {
		maxy -= (maxy - begy - tctx->lines);
	}

	/*fprintf(stderr, "-OFFSET BY %d (%d->%d)\n", maxy - begy - tctx->lines,
	  maxy, maxy - (maxy - begy - tctx->lines));*/
	for (y = maxy ; y >= begy ; --y, rgb += 16) {
		snprintf (cchbuf, sizeof(cchbuf) / sizeof(*cchbuf), "%x", idx % 16);
		w->load (c, cchbuf);
		if (!c.set_fg_rgb ((rgb >> 16u) % 0xffu, (rgb >> 8u) % 0xffu, rgb % 0xffu)) {
			return -1;
		}

		int x;
		for (x = begx ; x <= maxx ; ++x) {
			// lower-right corner always returns an error unless scrollok() is used
			if (w->putc (y, x, c) <= 0) {
				return -1;
			}
		}
		w->release (c);
		if (--idx == 0) {
			break;
		}
	}

	// fprintf(stderr, "tabletup done%s at %d (%d->%d)\n", idx == 0 ? " early" : "", y, begy, maxy);
	return tctx->lines - idx;
}

static int
tabletdown (Plane *w, int begx, int begy, int maxx, int maxy, tabletctx* tctx, unsigned rgb)
{
	char cchbuf[2];
	Cell c;
	int y;

	for (y = begy ; y <= maxy ; ++y, rgb += 16) {
		if (y - begy >= tctx->lines) {
			break;
		}

		snprintf (cchbuf, sizeof(cchbuf) / sizeof(*cchbuf), "%x", y % 16);
		w->load (c, cchbuf);
		if (!c.set_fg_rgb ((rgb >> 16u) % 0xffu, (rgb >> 8u) % 0xffu, rgb % 0xffu)) {
			return -1;
		}

		int x;
		for (x = begx ; x <= maxx ; ++x) {
			// lower-right corner always returns an error unless scrollok() is used
			if (w->putc (y, x, c) <= 0) {
				return -1;
			}
		}
		w->release (c);
	}

	return y - begy;
}

static int
tabletdraw (struct tablet* _t, int begx, int begy, int maxx, int maxy, bool cliptop)
{
	Tablet *t = Tablet::map_tablet (_t);
	Plane *p = t->get_plane ();
	tabletctx* tctx = t->get_userptr<tabletctx> ();

	pthread_mutex_lock (&tctx->lock);
	unsigned rgb = tctx->rgb;
	int ll;

	if (cliptop) {
		ll = tabletup (p, begx, begy, maxx, maxy, tctx, rgb);
	} else {
		ll = tabletdown (p, begx, begy, maxx, maxy, tctx, rgb);
	}
	p->set_fg_rgb (242, 242, 242);
	if (ll) {
		int summaryy = begy;
		if (cliptop) {
			if (ll == maxy - begy + 1) {
				summaryy = ll - 1;
			} else {
				summaryy = ll;
			}
		}

		p->styles_on (CellStyle::Bold);
		if (p->printf (summaryy, begx, "[#%u %d line%s %u/%u] ", tctx->id, tctx->lines, tctx->lines == 1 ? "" : "s", begy, maxy) < 0) {
			pthread_mutex_unlock (&tctx->lock);
			return -1;
		}
		p->styles_off (CellStyle::Bold);
	}
	/*fprintf(stderr, "  \\--> callback for %d, %d lines (%d/%d -> %d/%d) dir: %s wrote: %d ret: %d\n", tctx->id,
	  tctx->lines, begy, begx, maxy, maxx,
	  cliptop ? "up" : "down", ll, err);*/
	pthread_mutex_unlock (&tctx->lock);

	return ll;
}

// Each tablet has an associated thread which will periodically send update
// events for its tablet.
static void*
tablet_thread (void* vtabletctx)
{
	static int MINSECONDS = 0;
	auto tctx = static_cast<tabletctx*>(vtabletctx);
	while (true) {
		struct timespec ts;
		ts.tv_sec = random () % 2 + MINSECONDS;
		ts.tv_nsec = random () % 1000000000;
		nanosleep (&ts, nullptr);
		int action = random () % 5;
		pthread_mutex_lock (&tctx->lock);
		if (action < 2) {
			if ((tctx->lines -= (action + 1)) < 1) {
				tctx->lines = 1;
			}
			tctx->pr->touch (*tctx->t);
		} else if (action > 2) {
			if ((tctx->lines += (action - 2)) < 1) {
				tctx->lines = 1;
			}
			tctx->pr->touch (*tctx->t);
		}
		pthread_mutex_unlock (&tctx->lock);
	}

	return tctx;
}

static tabletctx*
new_tabletctx (std::shared_ptr<PanelReel> pr, unsigned *id)
{
	auto tctx = new tabletctx;
	if (tctx == nullptr) {
		return nullptr;
	}

	pthread_mutex_init (&tctx->lock, nullptr);
	tctx->pr = pr;
	tctx->lines = random () % 10 + 1; // FIXME a nice gaussian would be swell
	tctx->rgb = random () % (1u << 24u);
	tctx->id = ++*id;

	tctx->t.reset (pr->add (nullptr, nullptr, tabletdraw, tctx));
	if (!tctx->t) {
		pthread_mutex_destroy(&tctx->lock);
		delete tctx;
		return nullptr;
	}

	if (pthread_create (&tctx->tid, nullptr, tablet_thread, tctx)) {
		pthread_mutex_destroy(&tctx->lock);
		delete tctx;
		return nullptr;
	}

	return tctx;
}

static wchar_t
handle_input (NotCurses &nc, std::shared_ptr<PanelReel> pr, int efd, const struct timespec* deadline)
{
	struct pollfd fds[2] = {
		{ /*.fd =*/ STDIN_FILENO, /*.events =*/ POLLIN, /*.revents =*/ 0, },
		{ /*.fd =*/ efd,          /*.events =*/ POLLIN, /*.revents =*/ 0, },
	};
	sigset_t sset;
	sigemptyset (&sset);

	wchar_t key = -1;
	int pret;
	demo_render (nc);
	do {
		struct timespec pollspec, cur;
		clock_gettime (CLOCK_MONOTONIC, &cur);
		timespec_subtract (&pollspec, deadline, &cur);
		pret = ppoll (fds, sizeof(fds) / sizeof(*fds), &pollspec, &sset);
		if (pret == 0) {
			return 0;
		} else if (pret < 0) {
			std::cerr << "Error polling on stdin/eventfd (" << strerror (errno) << ")" << std::endl;
			return (wchar_t)-1;
		} else {
			if (fds[0].revents & POLLIN) {
				key = demo_getc_blocking (nullptr);
				if (key < 0) {
					return (wchar_t)-1;
				}
			}

			if (fds[1].revents & POLLIN) {
				uint64_t val;
				if (read (efd, &val, sizeof(val)) != sizeof(val)) {
					std::cerr << "Error reading from eventfd " << efd << " (" << strerror (errno) << ")" << std::endl;
				} else if (key < 0) {
					pr->redraw ();
					demo_render (nc);
				}
			}
		}
	} while (key < 0);

	return key;
}

static int
close_pipes (int* pipes)
{
	if (close (pipes[0]) | close (pipes[1])) { // intentional, avoid short-circuiting
		return -1;
	}
	return 0;
}

static std::shared_ptr<PanelReel>
panelreel_demo_core (NotCurses &nc, int efdr, int efdw, tabletctx** tctxs)
{
	bool done = false;
	int x = 8, y = 4;
	panelreel_options popts {};
	popts.infinitescroll = true;
	popts.circular = true;
	popts.min_supported_cols = 8;
	popts.min_supported_rows = 5;
	popts.bordermask = 0;
	popts.tabletmask = 0;
	popts.borderchan = 0;
	popts.tabletchan = 0;
	popts.focusedchan = 0;
	popts.toff = y;
	popts.loff = x;
	popts.roff = x;
	popts.boff = y;
	popts.bgchannel = 0;

	channels_set_fg_rgb (&popts.focusedchan, 58, 150, 221);
	channels_set_bg_rgb (&popts.focusedchan, 97, 214, 214);
	channels_set_fg_rgb (&popts.tabletchan, 19, 161, 14);
	channels_set_fg_rgb (&popts.borderchan, 136, 23, 152);
	channels_set_bg_rgb (&popts.borderchan, 0, 0, 0);
	if (channels_set_fg_alpha (&popts.bgchannel, Cell::AlphaTransparent)) {
		return nullptr;
	}

	if (channels_set_bg_alpha (&popts.bgchannel, Cell::AlphaTransparent)) {
		return nullptr;
	}

	Plane* w = nc.get_stdplane ();
	std::shared_ptr<PanelReel> pr (w->panelreel_create (&popts, efdw));
	if (!pr) {
		std::cerr << "Error creating panelreel" << std::endl;
		return nullptr;
	}

	// Press a for a new panel above the current, c for a new one below the
	// current, and b for a new block at arbitrary placement.
	w->styles_on (CellStyle::Bold | CellStyle::Italic);
	w->set_fg_rgb (58, 150, 221);
	w->set_bg_default ();
	w->printf (1, 1, "a, b, c create tablets, DEL deletes.");

	w->styles_off (CellStyle::Bold | CellStyle::Italic);
	// FIXME clrtoeol();

	struct timespec deadline;
	clock_gettime (CLOCK_MONOTONIC, &deadline);
	ns_to_timespec ((timespec_to_ns (&demodelay) * 5) + timespec_to_ns (&deadline), &deadline);

	unsigned id = 0;
	struct tabletctx* newtablet;
	int dimy = w->get_dim_y ();
	// Make an initial number of tablets suitable for the screen's height
	while (id < dimy / 8u) {
		newtablet = new_tabletctx (pr, &id);
		if (newtablet == nullptr) {
			return nullptr;
		}
		newtablet->next = *tctxs;
		*tctxs = newtablet;
	}

	do {
		w->styles_set (static_cast<CellStyle>(0));
		w->set_fg_rgb (197, 15, 31);
		int count = pr->get_tabletcount ();
		w->styles_on (CellStyle::Bold);
		w->printf (2, 2, "%d tablet%s", count, count == 1 ? "" : "s");
		w->styles_off (CellStyle::Bold);
		// FIXME wclrtoeol(w);
		w->set_fg_rgb (0, 55, 218);
		wchar_t rw;
		if ((rw = handle_input (nc, pr, efdr, &deadline)) <= 0) {
			done = true;
			break;
		}

		// FIXME clrtoeol();
		newtablet = nullptr;
		switch (rw) {
			case 'a': newtablet = new_tabletctx (pr, &id); break;
			case 'b': newtablet = new_tabletctx (pr, &id); break;
			case 'c': newtablet = new_tabletctx (pr, &id); break;
			case 'h': --x; if (!pr->move (x, y)) { ++x; } break;
			case 'l': ++x; if (!pr->move (x, y)) { --x; } break;
			case 'k': pr->prev (); break;
			case 'j': pr->next (); break;
			case 'q': done = true; break;
			case NCKey::Left: --x; if (!pr->move (x, y)) { ++x; } break;
			case NCKey::Right: ++x; if (!pr->move (x, y)) { --x; } break;
			case NCKey::Up: pr->prev (); break;
			case NCKey::Down: pr->next (); break;
			case NCKey::Del: kill_active_tablet (pr, tctxs); break;
			default:
				w->printf (3, 2, "Unknown keycode (0x%x)\n", rw);
		}

		if (newtablet) {
			newtablet->next = *tctxs;
			*tctxs = newtablet;
		}

		struct timespec cur;
		clock_gettime (CLOCK_MONOTONIC, &cur);
		if (timespec_subtract_ns (&cur, &deadline) >= 0){
			break;
		}
		//panelreel_validate(w, pr); // do what, if not assert()ing? FIXME
	} while(!done);

	return pr;
}

bool panelreel_demo (NotCurses &nc)
{
	tabletctx* tctxs = nullptr;
	int pipes[2];
	// freebsd doesn't have eventfd :/
	if (pipe2 (pipes, O_CLOEXEC | O_NONBLOCK)) {
		std::cerr << "Error creating pipe " << strerror (errno) << std::endl;
		return false;
	}

	std::shared_ptr<PanelReel> pr;
	if ((pr = panelreel_demo_core (nc, pipes[0], pipes[1], &tctxs)) == nullptr) {
		close_pipes (pipes);
		return false;
	}

	while (tctxs) {
		kill_tablet (&tctxs);
	}

	close_pipes (pipes);
	if (!pr->destroy ()) {
		std::cerr << "Error destroying panelreel" << std::endl;
		return false;
	}

	if (!demo_render (nc)) {
		return false;
	}

	return true;
}
