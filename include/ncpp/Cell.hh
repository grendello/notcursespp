#ifndef __NCPP_CELL_HH
#define __NCPP_CELL_HH

#include <map>
#include <mutex>
#include <notcurses.h>

#include "Root.hh"

namespace ncpp
{
	class NCPP_API_EXPORT Cell : public Root
	{
	public:
		explicit Cell () noexcept
		{}

		operator cell* () noexcept
		{
			return &_cell;
		}

		operator cell const* () const noexcept
		{
			return &_cell;
		}

	protected:
		cell* get_cell () noexcept
		{
			return &_cell;
		}

	private:
		cell _cell;
	};
}

#endif
