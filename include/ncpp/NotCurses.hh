#ifndef __NCPP_NOTCURSES_HH
#define __NCPP_NOTCURSES_HH

#include <cstdio>
#include <ctime>
#include <csignal>
#include <atomic>
#include <map>
#include <mutex>

#include <notcurses.h>

#include "CellStyle.hh"
#include "Plane.hh"
#include "_helpers.hh"

namespace std
{
	class mutex;
}

namespace ncpp
{
	class NCPP_API_EXPORT NotCurses
	{
	public:
		static notcurses_options default_notcurses_options;

	public:
		static NotCurses& get_instance () noexcept
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

		wchar_t getc (const timespec *ts, sigset_t *sigmask) const noexcept
		{
			return notcurses_getc (nc, ts, sigmask);
		}

		wchar_t getc (bool blocking = false) const noexcept
		{
			if (blocking)
				return notcurses_getc_blocking (nc);

			return notcurses_getc_nblock (nc);
		}

		Plane* get_stdplane () noexcept
		{
			if (stdplane == nullptr) {
				stdplane = new Plane (notcurses_stdplane (nc));
			}

			return stdplane;
		}

		Plane* get_top () noexcept;

	protected:
		NotCurses () noexcept
		{}

		~NotCurses () noexcept;

	private:
		notcurses *nc;
		Plane *stdplane = nullptr;
		std::atomic_bool initialized;
		std::map<ncplane*, Plane*> *top_planes = nullptr;
		std::mutex top_planes_mutex;

		static NotCurses _instance;
	};
}

#endif
