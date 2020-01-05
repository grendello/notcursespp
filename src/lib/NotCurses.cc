#include <config.hh>

#include <notcurses.h>

#include <ncpp/NotCurses.hh>
#include <ncpp/NCLogLevel.hh>

using namespace ncpp;

NotCurses NotCurses::_instance;

notcurses_options NotCurses::default_notcurses_options = {
	/* termtype */                 nullptr,
	/* inhibit_alternate_screen */ false,
	/* retain_cursor */            false,
	/* clear_screen_start */       false,
	/* suppress_bannner */         false,
	/* no_quit_sighandlers */      false,
	/* no_winch_sighandler */      false,
	/* renderfp */                 nullptr,
	/* loglevel */                 NCLogLevel::Silent,
};

NotCurses::~NotCurses () noexcept
{
	if (nc == nullptr)
		return;

	stop ();
}

bool NotCurses::init (const notcurses_options &nc_opts, FILE *fp) noexcept
{
	if (!initialized) {
		nc = notcurses_init (&nc_opts, fp == nullptr ? stdout : fp);
		initialized = true;
	}

	return nc != nullptr;
}

Plane* NotCurses::get_top () noexcept
{
	ncplane *top = notcurses_top (nc);
	if (top == nullptr)
		return nullptr;

	return Plane::map_plane (top);
}
