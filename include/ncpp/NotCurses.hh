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
#include "NCKey.hh"
#include "Plane.hh"
#include "_helpers.hh"

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

		operator bool () const noexcept
		{
			return nc != nullptr;
		}

		operator notcurses* () noexcept
		{
			return nc;
		}

		operator notcurses const* () const noexcept
		{
			return nc;
		}

		bool init (const notcurses_options &nc_opts, FILE *fp = nullptr) noexcept;

		bool init (FILE *fp = nullptr) noexcept
		{
			return init (default_notcurses_options, fp);
		}

		static const char* enmetric (uintmax_t val, unsigned decimal, char *buf, int omitdec, unsigned mult, int uprefix) noexcept
		{
			return ::enmetric (val, decimal, buf, omitdec, mult, uprefix);
		}

		static const char* qprefix (uintmax_t val, unsigned decimal, char *buf, int omitdec) noexcept
		{
			return ::qprefix (val, decimal, buf, omitdec);
		}

		static const char* bprefix (uintmax_t val, unsigned decimal, char *buf, int omitdec) noexcept
		{
			return ::bprefix (val, decimal, buf, omitdec);
		}

		static const char* version () noexcept
		{
			return notcurses_version ();
		}

		bool can_fade () const noexcept
		{
			return notcurses_canfade (nc);
		}

		bool can_open () const noexcept
		{
			return notcurses_canopen (nc);
		}

		void get_stats (ncstats *stats) const noexcept
		{
			if (stats == nullptr)
				return;

			notcurses_stats (nc, stats);
		}

		void reset_stats (ncstats *stats) const noexcept
		{
			if (stats == nullptr)
				return;

			notcurses_reset_stats (nc, stats);
		}

		bool render () const noexcept
		{
			return notcurses_render (nc) == 0;
		}

		bool resize (int *rows, int *cols) const noexcept
		{
			return notcurses_resize (nc, rows, cols) == 0;
		}

		bool resize (int &rows, int &cols) const noexcept
		{
			return resize (&rows, &cols) == 0;
		}

		void get_term_dim (int *rows, int *cols) const noexcept
		{
			notcurses_term_dim_yx (nc, rows, cols);
		}

		void get_term_dim (int &rows, int &cols) const noexcept
		{
			get_term_dim (&rows, &cols);
		}

		bool refresh () const noexcept
		{
			return notcurses_refresh (nc) == 0;
		}

		int get_palette_size () const noexcept
		{
			return notcurses_palette_size (static_cast<const notcurses*> (nc));
		}

		bool mouse_enable () const noexcept
		{
			return notcurses_mouse_enable (nc) != -1;
		}

		bool mouse_disable () const noexcept
		{
			return notcurses_mouse_disable (nc) != -1;
		}

		CellStyle get_supported_styles () const noexcept
		{
			return static_cast<CellStyle>(notcurses_supported_styles (nc));
		}

		char32_t getc (const timespec *ts, sigset_t *sigmask = nullptr, ncinput *ni = nullptr) const noexcept
		{
			return notcurses_getc (nc, ts, sigmask, ni);
		}

		char32_t getc (bool blocking = false, ncinput *ni = nullptr) const noexcept
		{
			if (blocking)
				return notcurses_getc_blocking (nc, ni);

			return notcurses_getc_nblock (nc, ni);
		}

		char* get_at (int yoff, int xoff, Cell &c) const noexcept
		{
			return notcurses_at_yx (nc, yoff, xoff, c);
		}

		Plane* get_stdplane () noexcept
		{
			return new Plane (notcurses_stdplane (nc), true);
		}

		bool stop () noexcept
		{
			if (nc == nullptr)
				return false;

			bool ret = notcurses_stop (nc) != -1;
			nc = nullptr;

			return ret;
		}

		Plane* get_top () noexcept;

	protected:
		NotCurses () noexcept
		{}

		~NotCurses () noexcept;

	private:
		notcurses *nc;
		std::atomic_bool initialized;
		std::map<ncplane*, Plane*> *top_planes = nullptr;
		std::mutex top_planes_mutex;

		static NotCurses _instance;
	};
}

#endif
