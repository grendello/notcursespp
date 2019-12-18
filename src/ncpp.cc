#include <config.hh>

#include <ncpp/ncpp.hh>

notcurses_options ncpp::default_notcurses_options = {
	/* termtype */                 nullptr,
	/* inhibit_alternate_screen */ false,
	/* retain_cursor */            false,
	/* no_quit_sighandlers */      false,
	/* no_winch_sighandler */      false,
	/* renderfp */                 nullptr
};
