#ifndef __NCPP_ROOT_HH
#define __NCPP_ROOT_HH

#include <notcurses.h>

#include "_helpers.hh"

namespace ncpp {
	class NCPP_API_EXPORT Root
	{
	protected:
		notcurses* get_notcurses () const noexcept;
	};
}
#endif
