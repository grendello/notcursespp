#include <config.hh>

#include "ncpp/Root.hh"
#include "ncpp/NotCurses.hh"

using namespace ncpp;

notcurses* Root::get_notcurses () const noexcept
{
	return NotCurses::get_instance ();
}
