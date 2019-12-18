#ifndef __NCPP_NOTCURSES_HH
#define __NCPP_NOTCURSES_HH

#include <cstdio>
#include <atomic>

#include <notcurses.h>

#include "CellStyle.hh"
#include "_helpers.hh"

namespace ncpp
{
	extern notcurses_options default_notcurses_options;

	class NCPP_API_EXPORT NotCurses
	{
	public:
		NotCurses& get_instance () const noexcept
		{
			return _instance;
		}

		bool init (const notcurses_options &nc_opts, FILE *fp = nullptr) noexcept;

		bool init (FILE *fp = nullptr) noexcept
		{
			return init (default_notcurses_options, fp);
		}

		notcurses* get_notcurses () const noexcept
		{
			return nc;
		}

		bool render () const noexcept
		{
			return notcurses_render (nc) == 0;
		}

		bool resize (int *rows, int *cols) const noexcept
		{
			return notcurses_resize (nc, rows, cols) == 0;
		}

		bool refresh () const noexcept
		{
			return notcurses_refresh (nc) == 0;
		}

		int get_palette_size () const noexcept
		{
			return notcurses_palette_size (static_cast<const notcurses*> (nc));
		}

		CellStyle get_supported_styles () const noexcept
		{
			return static_cast<CellStyle>(notcurses_supported_styles (nc));
		}

	protected:
		NotCurses () noexcept
		{}

		~NotCurses () noexcept;

	private:
		notcurses *nc;
		std::atomic_bool initialized;

		static NotCurses _instance;
	};
}

#endif
