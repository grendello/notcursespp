#include <config.hh>

#include <ctime>
#include <wchar.h>
#include <cstdio>
#include <climits>
#include <string.h>
#include <clocale>
#include <unistd.h>
#include <getopt.h>
#include <cstdlib>
#include <atomic>
#include <iostream>

#include "demo.hh"

#include <ncpp/NotCurses.hh>

using namespace ncpp;

// ansi terminal definition-4-life
static constexpr int MIN_SUPPORTED_ROWS = 24;
static constexpr int MIN_SUPPORTED_COLS = 80;

static std::atomic_bool interrupted (false);

static constexpr char DEFAULT_DEMO[] = "ixemlubgswvpo";
static char datadir[PATH_MAX] = NOTCURSES_DATA_DIR;

void interrupt_demo (void)
{
	interrupted.store (true);
}

char* find_data (const char* datum)
{
	char* path = new char[strlen (datadir) + 1 + strlen (datum) + 1];
	strcpy (path, datadir);
	strcat (path, "/");
	strcat (path, datum);
	return path;
}

int timespec_subtract (struct timespec *result, const struct timespec *time0, struct timespec *time1)
{
	if (time0->tv_nsec < time1->tv_nsec) {
		int nsec = (time1->tv_nsec - time0->tv_nsec) / 1000000000 + 1;
		time1->tv_nsec -= 1000000000 * nsec;
		time1->tv_sec += nsec;
	}

	if (time0->tv_nsec - time1->tv_nsec > 1000000000) {
		int nsec = (time0->tv_nsec - time1->tv_nsec) / 1000000000;
		time1->tv_nsec += 1000000000 * nsec;
		time1->tv_sec -= nsec;
	}

	result->tv_sec = time0->tv_sec - time1->tv_sec;
	result->tv_nsec = time0->tv_nsec - time1->tv_nsec;
	return time0->tv_sec < time1->tv_sec;
}

struct timespec demodelay = {
	/* tv_sec */  1,
	/* tv_nsec */ 0,
};

static void
usage (const char* exe, int status)
{
	std::ostream &out = status == EXIT_SUCCESS ? std::cout : std::cerr;
	out << "usage: " << exe << " [ -h ] [ -k ] [ -d mult ] [ -c ] [ -f renderfile ] demospec" << std::endl;
	out << " -h: this message" << std::endl;
	out << " -k: keep screen; do not switch to alternate" << std::endl;
	out << " -d: delay multiplier (float)" << std::endl;
	out << " -f: render to file in addition to stdout" << std::endl;
	out << " -c: constant PRNG seed, useful for benchmarking" << std::endl;
	out << "all demos are run if no specification is provided" << std::endl;
	out << " b: run box" << std::endl;
	out << " e: run eagles" << std::endl;
	out << " g: run grid" << std::endl;
	out << " i: run intro" << std::endl;
	out << " l: run luigi" << std::endl;
	out << " m: run maxcolor" << std::endl;
	out << " o: run outro" << std::endl;
	out << " p: run panelreels" << std::endl;
	out << " s: run shuffle" << std::endl;
	out << " u: run uniblock" << std::endl;
	out << " v: run view" << std::endl;
	out << " w: run witherworm" << std::endl;
	out << " x: run x-ray" << std::endl;
	exit (status);
}

static bool
intro (NotCurses &nc)
{
	Plane* ncp;
	if ((ncp = nc.get_stdplane ()) == nullptr) {
		return false;
	}
	Cell c;
	c.set_bg_rgb (0x20, 0x20, 0x20);
	ncp->set_default (c);
	if (!ncp->cursor_move_yx (0, 0)) {
		return false;
	}

	int x, y, rows, cols;
	ncp->get_dim (&rows, &cols);

	Cell ul, ur;
	Cell ll, lr;
	Cell hl, vl;
	if (!ncp->load_rounded_box (CellStyle::Bold, 0, ul, ur, ll, lr, hl, vl)) {
		return false;
	}

	ul.set_fg_rgb (0xff, 0, 0);
	ur.set_fg_rgb (0, 0xff, 0);
	ll.set_fg_rgb (0, 0, 0xff);
	lr.set_fg_rgb (0xff, 0xff, 0xff);
	if (!ncp->box_sized (ul, ur, ll, lr, hl, vl, rows, cols, NCBox::GradTop | NCBox::GradBottom | NCBox::GradRight | NCBox::GradLeft)) {
		return false;
	}

	ncp->release (ul); ncp->release (ur);
	ncp->release (ll); ncp->release (lr);
	ncp->release (hl); ncp->release (vl);

	const char* cstr = "Δ";
	ncp->load (c, cstr);
	c.set_fg_rgb (200, 0, 200);

	int ys = 200 / (rows - 2);
	for (y = 5 ; y < rows - 6 ; ++y) {
		c.set_bg_rgb (0, y * ys, 0);
		for (x = 5 ; x < cols - 6 ; ++x) {
			if (!ncp->cursor_move_yx (y, x)) {
				return false;
			}

			if (ncp->putc (c) <= 0) {
				return false;
			}
		}
	}
	ncp->release (c);

	uint64_t channels = 0;
	// FIXME: add proper Channels API
	channels_set_fg_rgb (&channels, 90, 0, 90);
	channels_set_bg_rgb (&channels, 0, 0, 180);
	if (!ncp->cursor_move_yx (4, 4)) {
		return false;
	}

	if (!ncp->rounded_box (0, channels, rows - 6, cols - 6, 0)) {
		return false;
	}

	constexpr char s1[] = " Die Welt ist alles, was der Fall ist. ";
	constexpr char str[] = " Wovon man nicht sprechen kann, darüber muss man schweigen. ";

	if (!ncp->set_fg_rgb (192, 192, 192)) {
		return false;
	}

	if (!ncp->set_bg_rgb (0, 40, 0)) {
		return false;
	}

	if (ncp->putstr (rows / 2 - 2, NCAlign::Center, s1) != (int)strlen(s1)) {
		return false;
	}

	ncp->styles_on (CellStyle::Italic | CellStyle::Bold);
	if (ncp->putstr (rows / 2, NCAlign::Center, str) != (int)strlen(str)) {
		return false;
	}

	ncp->styles_off (CellStyle::Italic);
	ncp->set_fg_rgb (0xff, 0xff, 0xff);
	if (ncp->putstr (rows - 3, NCAlign::Center, "press q at any time to quit") < 0) {
		return false;
	}
	ncp->styles_off (CellStyle::Bold);

	constexpr wchar_t wstr[] = L"▏▁ ▂ ▃ ▄ ▅ ▆ ▇ █ █ ▇ ▆ ▅ ▄ ▃ ▂ ▁▕";
	if (ncp->putstr (rows / 2 - 5, NCAlign::Center, wstr) < 0) {
		return false;
	}

	if (rows < 45) {
		ncp->set_fg_rgb (0xc0, 0, 0x80);
		ncp->set_bg_rgb (0x20, 0x20, 0x20);
		ncp->styles_on (CellStyle::Blink); // heh FIXME replace with pulse
		if (ncp->putstr (2, NCAlign::Center, "demo runs best with at least 45 lines") < 0){
			return false;
		}
		ncp->styles_off (CellStyle::Blink); // heh FIXME replace with pulse
	}

	if (!nc.render ()) {
		return false;
	}

	nanosleep (&demodelay, nullptr);

	struct timespec fade = demodelay;
	ncp->fadeout (&fade);

	return true;
}

typedef struct demoresult {
	char selector;
	struct ncstats stats;
	uint64_t timens;
	bool failed;
} demoresult;

static demoresult*
ext_demos (NotCurses &nc, const char* demos)
{
	bool ret = true;
	auto results = new demoresult[strlen (demos)]();

	struct timespec start, now;
	clock_gettime (CLOCK_MONOTONIC, &start);
	uint64_t prevns = timespec_to_ns (&start);
	for (size_t i = 0 ; i < strlen (demos) ; ++i) {
		results[i].selector = demos[i];
	}

	for (size_t i = 0 ; i < strlen (demos) ; ++i) {
		if (interrupted) {
			break;
		}

		switch (demos[i]) {
			case 'i': ret = intro (nc); break;
			case 'o': ret = outro (nc); break;
			case 's': ret = sliding_puzzle_demo (nc); break;
			case 'u': ret = unicodeblocks_demo (nc); break;
			case 'm': ret = maxcolor_demo (nc); break;
			case 'b': ret = box_demo (nc); break;
			case 'g': ret = grid_demo (nc); break;
			case 'l': ret = luigi_demo (nc); break;
			case 'v': ret = view_demo (nc); break;
			case 'e': ret = eagle_demo (nc); break;
			case 'x': ret = xray_demo (nc); break;
			case 'w': ret = witherworm_demo (nc); break;
			case 'p': ret = panelreel_demo (nc); break;
			default:
				std::cerr << "Unknown demo specification: " << *demos << std::endl;
				ret = false;
				break;
		}

		nc.get_stats (&results[i].stats);
		nc.reset_stats ();
		clock_gettime (CLOCK_MONOTONIC, &now);
		uint64_t nowns = timespec_to_ns(&now);
		results[i].timens = nowns - prevns;
		prevns = nowns;

		if (!ret) {
			results[i].failed = true;
			break;
		}
	}
	return results;
}

// returns the demos to be run as a string. on error, returns NULL. on no
// specification, also returns NULL, heh. determine this by argv[optind];
// if it's NULL, there were valid options, but no spec.
static const char*
handle_opts (int argc, char** argv, notcurses_options* opts)
{
	bool constant_seed = false;
	int c;
	memset (opts, 0, sizeof(*opts));
	while ((c = getopt (argc, argv, "hckd:f:p:")) != EOF) {
		switch (c) {
			case 'h':
				usage (*argv, EXIT_SUCCESS);
				break;

			case 'c':
				constant_seed = true;
				break;

			case 'k':
				opts->inhibit_alternate_screen = true;
				break;

			case 'f':
				if (opts->renderfp) {
					std::cerr << "-f may only be supplied once" << std::endl;
					usage (*argv, EXIT_FAILURE);
				}

				if ((opts->renderfp = fopen (optarg, "wb")) == nullptr) {
					usage (*argv, EXIT_FAILURE);
				}
				break;

			case 'p':
				strcpy (datadir, optarg);
				break;

			case 'd': {
				float f;
				if (sscanf (optarg, "%f", &f) != 1) {
					std::cerr << "Couldn't get a float from " << optarg << std::endl;
					usage (*argv, EXIT_FAILURE);
				}

				uint64_t ns = f * GIG;
				demodelay.tv_sec = ns / GIG;
				demodelay.tv_nsec = ns % GIG;
				break;
			}

			default:
				usage (*argv, EXIT_FAILURE);
		}
	}

	if (!constant_seed){
		srand (time (nullptr)); // a classic blunder lol
	}

	const char* demos = argv[optind];
	return demos;
}

// just fucking around...for now
int main (int argc, char** argv)
{
	NotCurses &nc = NotCurses::get_instance ();
	notcurses_options nopts;

	if (!setlocale (LC_ALL, "")) {
		std::cerr << "Couldn't set locale based on user preferences" << std::endl;
		return EXIT_FAILURE;
	}

	const char* demos;
	if ((demos = handle_opts (argc, argv, &nopts)) == nullptr) {
		if(argv[optind] != nullptr) {
			usage (*argv, EXIT_FAILURE);
		}
		demos = DEFAULT_DEMO;
	}

	bool failed;
	demoresult *results = nullptr;
	int dimx, dimy;

	nc.init (nopts);
	if (!nc) {
		goto err;
	}

	if (!nc.mouse_enable ()) {
		goto err;
	}

	if (!input_dispatcher (nc)) {
		goto err;
	}

	nc.term_dim_yx (&dimy, &dimx);
	if (dimy < MIN_SUPPORTED_ROWS || dimx < MIN_SUPPORTED_COLS) {
		goto err;
	}

	// no one cares about the leaderscreen. 1s max.
	if (demodelay.tv_sec >= 1) {
		sleep (1);
	} else {
		nanosleep (&demodelay, nullptr);
	}

	results = ext_demos (nc, demos);
	if (results == nullptr) {
		goto err;
	}

	if (!nc.stop ()) {
		return EXIT_FAILURE;
	}

	failed = false;
	for (size_t i = 0 ; i < strlen (demos) ; ++i) {
		char totalbuf[BPREFIXSTRLEN + 1];
		bprefix (results[i].stats.render_bytes, 1, totalbuf, 0);
		double avg = results[i].stats.render_ns / (double)results[i].stats.renders;
		printf ("%2zu|%c|%2lu.%03lus|%4luf|%*sB|%8juµs|%6.1f FPS|%s\n", i,
				results[i].selector,
				results[i].timens / GIG,
				(results[i].timens % GIG) / 1000000,
				results[i].stats.renders,
				BPREFIXSTRLEN, totalbuf,
				results[i].stats.render_ns / 1000,
				GIG / avg,
				results[i].failed ? "***FAILED" : results[i].stats.renders ? ""  : "***NOT RUN");
		if (results[i].failed) {
			failed = true;
		}
	}

	delete results;

	if (failed) {
		std::cerr << " Error running demo. Did you need provide -p?" << std::endl;
	}
	return failed ? EXIT_FAILURE : EXIT_SUCCESS;

  err:
	nc.term_dim_yx (&dimy, &dimx);
	nc.stop ();
	if (dimy < MIN_SUPPORTED_ROWS || dimx < MIN_SUPPORTED_COLS) {
		std::cerr << "At least an 80x25 terminal is required (current: " << dimx << "x" << dimy << std::endl;
	}

	return EXIT_FAILURE;
}
