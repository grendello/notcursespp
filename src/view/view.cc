#include <config.hh>

#include <array>
#include <cstdlib>
#include <clocale>
#include <libgen.h>
#include <unistd.h>
#include <iostream>

extern "C" {
#include <libavutil/pixdesc.h>
#include <libavutil/avconfig.h>
#include <libavcodec/avcodec.h> // ffmpeg doesn't reliably "C"-guard itself
}

#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>

using namespace ncpp;

static void usage (std::ostream& os, const char* name, int exitcode)
	__attribute__ ((noreturn));

void usage (std::ostream& o, const char* name, int exitcode)
{
	o << "usage: " << name << " files" << std::endl;
	exit (exitcode);
}

constexpr auto NANOSECS_IN_SEC = 1000000000ll;

static inline uint64_t
timespec_to_ns (const struct timespec* ts)
{
  return ts->tv_sec * NANOSECS_IN_SEC + ts->tv_nsec;
}

// FIXME: make the callback use notcurses++ classes
// frame count is in the curry. original time is in the ncplane's userptr.
int perframe ([[maybe_unused]] struct notcurses* _nc, struct ncvisual* ncv, void* vframecount)
{
	NotCurses &nc = NotCurses::get_instance ();

	const struct timespec* start = static_cast<struct timespec*>(ncplane_userptr (ncvisual_plane (ncv)));
	Plane* stdn = nc.get_stdplane ();
	int* framecount = static_cast<int*>(vframecount);
	++*framecount;
	stdn->set_fg (0x80c080);
	stdn->cursor_move (0, 0);

	struct timespec now;
	clock_gettime (CLOCK_MONOTONIC, &now);
	int64_t ns = timespec_to_ns (&now) - timespec_to_ns (start);
	stdn->printf ("Got frame %05d\u2026", *framecount);
	const int64_t h = ns / (60 * 60 * NANOSECS_IN_SEC);
	ns -= h * (60 * 60 * NANOSECS_IN_SEC);
	const int64_t m = ns / (60 * NANOSECS_IN_SEC);
	ns -= m * (60 * NANOSECS_IN_SEC);
	const int64_t s = ns / NANOSECS_IN_SEC;
	ns -= s * NANOSECS_IN_SEC;
	stdn->printf (0, NCAlign::Right, "%02ld:%02ld:%02ld.%04ld", h, m, s, ns / 1000000);
	if (!nc.render ()) {
		return -1;
	}

	int dimx, dimy, oldx, oldy, keepy, keepx;
	nc.get_term_dim (&dimy, &dimx);
	ncplane_dim_yx (ncvisual_plane (ncv), &oldy, &oldx);
	keepy = oldy > dimy ? dimy : oldy;
	keepx = oldx > dimx ? dimx : oldx;
	return ncplane_resize (ncvisual_plane (ncv), 0, 0, keepy, keepx, 0, 0, dimy, dimx);
}

int main (int argc, char** argv)
{
	setlocale (LC_ALL, "");
	if (argc == 1) {
		usage (std::cerr, argv[0], EXIT_FAILURE);
	}
	NotCurses &nc = NotCurses::get_instance ();
	nc.init ();
	if (!nc) {
		return EXIT_FAILURE;
	}
	int dimy, dimx;
	nc.get_term_dim (&dimy, &dimx);

	int frames;
	Plane ncp (dimy - 1, dimx, 1, 0, &frames);
	if (ncp == nullptr) {
		return EXIT_FAILURE;
	}

	for (int i = 1 ; i < argc ; ++i) {
		std::array<char, 128> errbuf;
		int averr;
		frames = 0;
		Visual *ncv = ncp.visual_open (argv[i], &averr);
		if (ncv == nullptr) {
			av_make_error_string (errbuf.data (), errbuf.size (), averr);
			std::cerr << "Error opening " << argv[i] << ": " << errbuf.data () << std::endl;
			return EXIT_FAILURE;
		}

		if (!ncv->stream (&averr, perframe)) {
			av_make_error_string (errbuf.data (), errbuf.size (), averr);
			std::cerr << "Error decoding " << argv[i] << ": " << errbuf.data () << std::endl;
			return EXIT_FAILURE;
		}
		nc.getc (true);
		delete ncv;
	}

	return EXIT_SUCCESS;
}
