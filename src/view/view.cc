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

int ncview (NotCurses &nc, Visual *ncv, int *averr)
{
	Plane* n = nc.get_stdplane ();
	int frame = 1;
	AVFrame* avf;

	struct timespec start;
	// FIXME should keep a start time and cumulative time; this will push things
	// out on a loaded machine
	while (clock_gettime (CLOCK_MONOTONIC, &start),
		  (avf = ncv->decode (averr))) {
		n->cursor_move_yx (0, 0);
		n->printf ("Got frame %05d\u2026", frame);
		if (!ncv->render (0, 0, 0, 0)) {
			return -1;
		}
		if (!nc.render ()) {
			return -1;
		}
		++frame;
		uint64_t ns = avf->pkt_duration * 1000000;
		struct timespec interval = {
			/* tv_sec */  start.tv_sec + (long)(ns / 1000000000),
			/* tv_nsec */ start.tv_nsec + (long)(ns % 1000000000),
		};
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &interval, NULL);
	}
	if (*averr == AVERROR_EOF) {
		return 0;
	}

	return -1;
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
	nc.term_dim_yx (&dimy, &dimx);

	Plane ncp (dimy - 1, dimx, 1, 0);
	if (ncp == nullptr) {
		return EXIT_FAILURE;
	}

	for (int i = 1 ; i < argc ; ++i) {
		std::array<char, 128> errbuf;
		int averr;
		Visual *ncv = ncp.visual_open (argv[i], &averr);
		if (ncv == nullptr) {
			av_make_error_string (errbuf.data (), errbuf.size (), averr);
			std::cerr << "Error opening " << argv[i] << ": " << errbuf.data () << std::endl;
			return EXIT_FAILURE;
		}

		if (ncview (nc, ncv, &averr)) {
			av_make_error_string (errbuf.data (), errbuf.size (), averr);
			std::cerr << "Error decoding " << argv[i] << ": " << errbuf.data () << std::endl;
			return EXIT_FAILURE;
		}
		nc.getc (true);
		delete ncv;
	}

	return EXIT_SUCCESS;
}
