#include <config.hh>

#include <notcurses.h>

#include <ncpp/NotCurses.hh>

using namespace ncpp;

NotCurses NotCurses::_instance;

notcurses_options NotCurses::default_notcurses_options = {
	/* termtype */                 nullptr,
	/* inhibit_alternate_screen */ false,
	/* retain_cursor */            false,
	/* no_quit_sighandlers */      false,
	/* no_winch_sighandler */      false,
	/* renderfp */                 nullptr
};

NotCurses::~NotCurses () noexcept
{
	if (nc != nullptr)
		notcurses_stop (nc);
}

bool NotCurses::init (const notcurses_options &nc_opts, FILE *fp) noexcept
{
	if (!initialized) {
		nc = notcurses_init (&nc_opts, fp);
		initialized = true;
	}

	// TODO: get and stash instance of the standard plane here, wrapping it in
	//       the Plane class once we have it.
	return nc != nullptr;
}

Plane* NotCurses::get_top () noexcept
{
	ncplane *top = notcurses_top (nc);
	if (top == nullptr)
		return nullptr;

	return Plane::map_plane (top);
}
