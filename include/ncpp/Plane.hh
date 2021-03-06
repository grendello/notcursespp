#ifndef __NCPP_PLANE_HH
#define __NCPP_PLANE_HH

#include <exception>
#include <cstdarg>
#include <ctime>
#include <map>
#include <mutex>
#include <notcurses.h>

#include "Root.hh"
#include "Cell.hh"
#include "Visual.hh"
#include "PanelReel.hh"
#include "CellStyle.hh"
#include "NCAlign.hh"
#include "NCBox.hh"

namespace ncpp
{
	class NCPP_API_EXPORT Plane : public Root
	{
	public:
		explicit Plane (int rows, int cols, int yoff, int xoff, void *opaque = nullptr) noexcept
		{
			plane = ncplane_new (
				get_notcurses (),
				rows,
				cols,
				yoff,
				xoff,
				opaque
			);

			map_plane (plane, this);
		}

		explicit Plane (Plane &n, int rows, int cols, int yoff, NCAlign align, void *opaque = nullptr)
		{
			plane = create_plane (n, rows, cols, yoff, align, opaque);
		}

		explicit Plane (Plane const& n, int rows, int cols, int yoff, NCAlign align, void *opaque = nullptr)
		{
			plane = create_plane (const_cast<Plane&>(n), rows, cols, yoff, align, opaque);
		}

		explicit Plane (Plane *n, int rows, int cols, int yoff, NCAlign align, void *opaque = nullptr)
		{
			if (n == nullptr)
				return;

			plane = create_plane (*n, rows, cols, yoff, align, opaque);
		}

		explicit Plane (Plane const* n, int rows, int cols, int yoff, NCAlign align, void *opaque = nullptr)
		{
			if (n == nullptr)
				return;

			plane = create_plane (const_cast<Plane&>(*n), rows, cols, yoff, align, opaque);
		}

		explicit Plane (ncplane *plane) noexcept
			: plane (plane)
		{}

		~Plane () noexcept
		{
			destroy ();
		}

		operator ncplane* () noexcept
		{
			return plane;
		}

		operator ncplane const* () const noexcept
		{
			return plane;
		}

		operator bool () noexcept
		{
			return plane != nullptr;
		}

		void destroy () noexcept
		{
			if (plane == nullptr || is_stdplane || !get_notcurses ())
				return;

			ncplane_destroy (plane);
			unmap_plane (this);
			plane = nullptr;
		}

		bool resize (int keepy, int keepx, int keepleny, int keeplenx, int yoff, int xoff, int ylen, int xlen) const noexcept
		{
			return ncplane_resize (
				plane,
				keepy,
				keepx,
				keepleny,
				keeplenx,
				yoff,
				xoff,
				ylen,
				xlen
			) != -1;
		}

		bool resize (int ylen, int xlen) const noexcept
		{
			return ncplane_resize_simple (plane, ylen, xlen) != -1;
		}

		bool fadeout (timespec *ts, fadecb fader = nullptr) const noexcept
		{
			return fadeout (static_cast<const timespec*>(ts), fader);
		}

		bool fadeout (timespec &ts, fadecb fader = nullptr) const noexcept
		{
			return fadeout (&ts, fader);
		}

		bool fadeout (timespec const& ts, fadecb fader = nullptr) const noexcept
		{
			return fadeout (&ts, fader);
		}

		bool fadeout (const timespec *ts, fadecb fader = nullptr) const noexcept
		{
			return ncplane_fadeout (plane, ts, fader) != -1;
		}

		bool fadein (timespec *ts, fadecb fader = nullptr) const noexcept
		{
			return fadein (static_cast<const timespec*>(ts), fader);
		}

		bool fadein (timespec &ts, fadecb fader = nullptr) const noexcept
		{
			return fadein (&ts, fader);
		}

		bool fadein (timespec const& ts, fadecb fader = nullptr) const noexcept
		{
			return fadein (&ts, fader);
		}

		bool fadein (const timespec *ts, fadecb fader = nullptr) const noexcept
		{
			return ncplane_fadein (plane, ts, fader) != -1;
		}

		void erase () const noexcept
		{
			ncplane_erase (plane);
		}

		int get_align (NCAlign align, int c) const noexcept
		{
			return ncplane_align (plane, static_cast<ncalign_e>(align), c);
		}

		void get_dim (int *rows, int *cols) const noexcept
		{
			ncplane_dim_yx (plane, rows, cols);
		}

		void get_dim (int &rows, int &cols) const noexcept
		{
			get_dim (&rows, &cols);
		}

		int get_dim_x () const noexcept
		{
			return ncplane_dim_x (plane);
		}

		int get_dim_y () const noexcept
		{
			return ncplane_dim_y (plane);
		}

		void get_yx (int *y, int *x) const noexcept
		{
			ncplane_yx (plane, y, x);
		}

		void get_yx (int &y, int &x) const noexcept
		{
			get_yx (&y, &x);
		}

		bool move (int y, int x) const noexcept
		{
			return ncplane_move_yx (plane, y, x) != -1;
		}

		bool move_top () const noexcept
		{
			return ncplane_move_top (plane) != -1;
		}

		bool move_bottom () const noexcept
		{
			return ncplane_move_bottom (plane) != -1;
		}

		bool move_below (Plane &below) const noexcept
		{
			return ncplane_move_below (plane, below.plane) != -1;
		}

		bool move_below (Plane *below) const noexcept
		{
			if (below == nullptr)
				return false;

			return move_below (*below);
		}

		bool move_below_unsafe (Plane &below) const noexcept
		{
			return ncplane_move_below_unsafe (plane, below.plane) != -1;
		}

		bool move_below_unsafe (Plane *below) const noexcept
		{
			if (below == nullptr)
				return false;

			return move_below_unsafe (*below);
		}

		bool move_above (Plane &above) const noexcept
		{
			return ncplane_move_above (plane, above.plane) != -1;
		}

		bool move_above (Plane *above) const noexcept
		{
			if (above == nullptr)
				return false;

			return move_above (*above);
		}

		bool move_above_unsafe (Plane &above) const noexcept
		{
			return ncplane_move_above_unsafe (plane, above.plane) != -1;
		}

		bool move_above_unsafe (Plane *above) const noexcept
		{
			if (above == nullptr)
				return false;

			return move_above (*above);
		}

		bool cursor_move (int y, int x) const noexcept
		{
			return ncplane_cursor_move_yx (plane, y, x) != -1;
		}

		void get_cursor_yx (int *y, int *x) const noexcept
		{
			ncplane_cursor_yx (plane, y, x);
		}

		void get_cursor_yx (int &y, int &x) const noexcept
		{
			get_cursor_yx (&y, &x);
		}

		int putc (const Cell &c) const noexcept
		{
			return ncplane_putc (plane, c);
		}

		int putc (const Cell *c) const
		{
			if (c == nullptr)
				return -1;

			return putc (*c);
		}

		int putc (int y, int x, Cell const& c) const noexcept
		{
			return ncplane_putc_yx (plane, y, x, c);
		}

		int putc (int y, int x, Cell const* c) const noexcept
		{
			if (c == nullptr)
				return -1;

			return putc (y, x, *c);
		}

		int putc (char c) const noexcept
		{
			return ncplane_putsimple (plane, c);
		}

		int putc (int y, int x, char c) const noexcept
		{
			return ncplane_putsimple_yx (plane, y, x, c);
		}

		int putc (const char *gclust, int *sbytes = nullptr) const noexcept
		{
			return ncplane_putegc (plane, gclust, sbytes);
		}

		int putc (int y, int x, const char *gclust, int *sbytes = nullptr) const noexcept
		{
			return ncplane_putegc_yx (plane, y, x, gclust, sbytes);
		}

		int putc (const wchar_t *gclust, int *sbytes = nullptr) const noexcept
		{
			return ncplane_putwegc (plane, gclust, sbytes);
		}

		int putc (int y, int x, const wchar_t *gclust, int *sbytes = nullptr) const noexcept
		{
			return ncplane_putwegc_yx (plane, y, x, gclust, sbytes);
		}

		int putstr (const char *gclustarr) const noexcept
		{
			return ncplane_putstr (plane, gclustarr);
		}

		int putstr (int y, int x, const char *gclustarr) const noexcept
		{
			return ncplane_putstr_yx (plane, y, x, gclustarr);
		}

		int putstr (int y, NCAlign atype, const char *s) const noexcept
		{
			return ncplane_putstr_aligned (plane, y, static_cast<ncalign_e>(atype), s);
		}

		int putstr (const wchar_t *gclustarr) const noexcept
		{
			return ncplane_putwstr (plane, gclustarr);
		}

		int putstr (int y, int x, const wchar_t *gclustarr) const noexcept
		{
			return ncplane_putwstr_yx (plane, y, x, gclustarr);
		}

		int putstr (int y, NCAlign atype, const wchar_t *gclustattr) const noexcept
		{
			return ncplane_putwstr_aligned (plane, y, static_cast<ncalign_e>(atype), gclustattr);
		}

		int printf (const char* format, ...) const noexcept
			__attribute__ ((format (printf, 2, 3)))
		{
			va_list va;

			va_start (va, format);
			int ret = ncplane_vprintf (plane, format, va);
			va_end (va);

			return ret;
		}

		int printf (int y, int x, const char *format, ...) const noexcept
			__attribute__ ((format (printf, 4, 5)))
		{
			va_list va;

			va_start (va, format);
			int ret = ncplane_vprintf_yx (plane, y, x, format, va);
			va_end (va);

			return ret;
		}

		int printf (int y, NCAlign align, const char *format, ...) const noexcept
			__attribute__ ((format (printf, 4, 5)))
		{
			va_list va;

			va_start (va, format);
			int ret = vprintf (y, align, format, va);
			va_end (va);

			return ret;
		}

		int vprintf (const char* format, va_list ap) const noexcept
		{
			return ncplane_vprintf (plane, format, ap);
		}

		int vprintf (int y, int x, const char* format, va_list ap) const noexcept
		{
			return ncplane_vprintf_yx (plane, y, x, format, ap);
		}

		int vprintf (int y, NCAlign align, const char *format, va_list ap) const noexcept
		{
			return ncplane_vprintf_aligned (plane, y, static_cast<ncalign_e>(align), format, ap);
		}

		int hline (const Cell &c, int len) const noexcept
		{
			return ncplane_hline (plane, c, len);
		}

		int hline (const Cell &c, int len, uint64_t c1, uint64_t c2) const noexcept
		{
			return ncplane_hline_interp (plane, c, len, c1, c2);
		}

		int vline (const Cell &c, int len) const noexcept
		{
			return ncplane_vline (plane, c, len);
		}

		int vline (const Cell &c, int len, uint64_t c1, uint64_t c2) const noexcept
		{
			return ncplane_vline_interp (plane, c, len, c1, c2);
		}

		bool load_box (uint32_t attrs, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl, const char *gclusters) const noexcept
		{
			return cells_load_box (plane, attrs, channels, ul, ur, ll, lr, hl, vl, gclusters) != -1;
		}

		bool load_box (CellStyle style, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl, const char *gclusters) const noexcept
		{
			return load_box (static_cast<uint32_t>(style), channels, ul, ur, ll, lr, hl, vl, gclusters);
		}

		bool load_rounded_box (uint32_t attr, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl) const noexcept
		{
			return cells_rounded_box (plane, attr, channels, ul, ur, ll, lr, hl, vl) != -1;
		}

		bool load_rounded_box (CellStyle style, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl) const noexcept
		{
			return load_rounded_box (static_cast<uint32_t>(style), channels, ul, ur, ll, lr, hl, vl);
		}

		bool load_double_box (uint32_t attr, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl) const noexcept
		{
			return cells_double_box (plane, attr, channels, ul, ur, ll, lr, hl, vl) != -1;
		}

		bool load_double_box (CellStyle style, uint64_t channels, Cell &ul, Cell &ur, Cell &ll, Cell &lr, Cell &hl, Cell &vl) const noexcept
		{
			return load_double_box (static_cast<uint32_t>(style), channels, ul, ur, ll, lr, hl, vl);
		}

		bool box (const Cell &ul, const Cell &ur, const Cell &ll, const Cell &lr,
				  const Cell &hline, const Cell &vline, int ystop, int xstop,
				  unsigned ctlword) const noexcept
		{
			return ncplane_box (plane, ul, ur, ll, lr, hline, vline, ystop, xstop, ctlword) != -1;
		}

		bool box_sized (const Cell &ul, const Cell &ur, const Cell &ll, const Cell &lr,
						const Cell &hline, const Cell &vline, int ylen, int xlen,
						unsigned ctlword) const noexcept
		{
			return ncplane_box_sized (plane, ul, ur, ll, lr, hline, vline, ylen, xlen, ctlword) != -1;
		}

		bool rounded_box (uint32_t attr, uint64_t channels, int ystop, int xstop, unsigned ctlword) const noexcept
		{
			return ncplane_rounded_box (plane, attr, channels, ystop, xstop, ctlword) != -1;
		}

		bool rounded_box_sized (uint32_t attr, uint64_t channels, int ylen, int xlen, unsigned ctlword) const noexcept
		{
			return ncplane_rounded_box_sized (plane, attr, channels, ylen, xlen, ctlword) != -1;
		}

		bool double_box (uint32_t attr, uint64_t channels, int ystop, int xstop, unsigned ctlword) const noexcept
		{
			return ncplane_double_box (plane, attr, channels, ystop, xstop, ctlword) != -1;
		}

		bool double_box_sized (uint32_t attr, uint64_t channels, int ylen, int xlen, unsigned ctlword) const noexcept
		{
			return ncplane_double_box_sized (plane, attr, channels, ylen, xlen, ctlword) != -1;
		}

		uint64_t get_channels () const noexcept
		{
			return ncplane_channels (plane);
		}

		uint32_t get_attr () const noexcept
		{
			return ncplane_attr (plane);
		}

		unsigned get_bchannel () const noexcept
		{
			return ncplane_bchannel (plane);
		}

		unsigned get_fchannel () const noexcept
		{
			return ncplane_fchannel (plane);
		}

		unsigned get_fg () const noexcept
		{
			return ncplane_fg (plane);
		}

		unsigned get_bg () const noexcept
		{
			return ncplane_bg (plane);
		}

		unsigned get_fg_alpha () const noexcept
		{
			return ncplane_fg_alpha (plane);
		}

		bool set_fg_alpha (int alpha) const noexcept
		{
			return ncplane_set_fg_alpha (plane, alpha) != -1;
		}

		unsigned get_bg_alpha () const noexcept
		{
			return ncplane_bg_alpha (plane);
		}

		bool set_bg_alpha (int alpha) const noexcept
		{
			return ncplane_set_bg_alpha (plane, alpha) != -1;
		}

		unsigned get_fg_rgb (unsigned *r, unsigned *g, unsigned *b) const noexcept
		{
			return ncplane_fg_rgb (plane, r, g, b);
		}

		bool set_fg_rgb (unsigned r, unsigned g, unsigned b) const noexcept
		{
			return ncplane_set_fg_rgb (plane, r, g, b) != -1;
		}

		bool set_fg (uint32_t channel) const noexcept
		{
			return ncplane_set_fg (plane, channel) != -1;
		}

		void set_fg_default () const noexcept
		{
			ncplane_set_fg_default (plane);
		}

		unsigned get_bg_rgb (unsigned *r, unsigned *g, unsigned *b) const noexcept
		{
			return ncplane_bg_rgb (plane, r, g, b);
		}

		bool set_bg_rgb (unsigned r, unsigned g, unsigned b) const noexcept
		{
			return ncplane_set_bg_rgb (plane, r, g, b) != -1;
		}

		bool set_bg (uint32_t channel) const noexcept
		{
			return ncplane_set_bg (plane, channel) != -1;
		}

		void set_bg_default () const noexcept
		{
			ncplane_set_bg_default (plane);
		}

		void styles_set (CellStyle styles) const noexcept
		{
			ncplane_styles_set (plane, static_cast<unsigned>(styles));
		}

		void styles_on (CellStyle styles) const noexcept
		{
			ncplane_styles_on (plane, static_cast<unsigned>(styles));
		}

		void styles_off (CellStyle styles) const noexcept
		{
			ncplane_styles_off (plane, static_cast<unsigned>(styles));
		}

		Plane* get_below () const noexcept
		{
			return map_plane (ncplane_below (plane));
		}

		bool set_base (Cell &c) const noexcept
		{
			return ncplane_set_base (plane, c) >= 0;
		}

		bool get_base (Cell &c) const noexcept
		{
			return ncplane_base (plane, c) >= 0;
		}

		bool at_cursor (Cell &c) const noexcept
		{
			return ncplane_at_cursor (plane, c) >= 0;
		}

		bool at_cursor (Cell *c) const
		{
			if (c == nullptr)
				return false;

			return at_cursor (*c);
		}

		int get_at (int y, int x, Cell &c) const noexcept
		{
			return ncplane_at_yx (plane, y, x, c);
		}

		int get_at (int y, int x, Cell *c) const
		{
			if (c == nullptr)
				return -1;

			return get_at (y, x, *c);
		}

		void* set_userptr (void *opaque) const noexcept
		{
			return ncplane_set_userptr (plane, opaque);
		}

		template<typename T>
		T* set_userptr (T *opaque) const noexcept
		{
			return static_cast<T*>(set_userptr (static_cast<void*>(opaque)));
		}

		void* get_userptr () const noexcept
		{
			return ncplane_userptr (plane);
		}

		template<typename T>
		T* get_userptr () const noexcept
		{
			return static_cast<T*>(get_userptr ());
		}

		Visual* visual_open (const char *file, int *averr) const noexcept
		{
			return new Visual (plane, file, averr);
		}

		PanelReel* panelreel_create (const panelreel_options *popts = nullptr, int efd = -1) const noexcept
		{
			return new PanelReel (plane, popts, efd);
		}

		// Some Cell APIs go here since they act on individual panels even though it may seem weird at points (e.g.
		// release)

		int load (Cell &cell, const char *gcluster) const noexcept
		{
			return cell_load (plane, cell, gcluster);
		}

		bool load (Cell &cell, char ch) const noexcept
		{
			return cell_load_simple (plane, cell, ch) != -1;
		}

		int prime (Cell &cell, const char *gcluster, uint32_t attr, uint64_t channels) const noexcept
		{
			return cell_prime (plane, cell, gcluster, attr, channels);
		}

		void release (Cell &cell) const noexcept
		{
			cell_release (plane, cell);
		}

		int duplicate (Cell &target, Cell &source) const noexcept
		{
			return cell_duplicate (plane, target, source);
		}

		int duplicate (Cell &target, Cell const& source) const noexcept
		{
			return cell_duplicate (plane, target, source);
		}

		int duplicate (Cell &target, Cell *source) const
		{
			if (source == nullptr)
				return -1;

			return duplicate (target, *source);
		}

		int duplicate (Cell &target, Cell const* source) const
		{
			if (source == nullptr)
				return -1;

			return duplicate (target, *source);
		}

		int duplicate (Cell *target, Cell *source) const
		{
			if (target == nullptr || source == nullptr)
				return -1;

			return duplicate (*target, *source);
		}

		int duplicate (Cell *target, Cell const* source) const
		{
			if (target == nullptr || source == nullptr)
				return -1;

			return duplicate (*target, *source);
		}

		int duplicate (Cell *target, Cell &source) const
		{
			if (target == nullptr)
				return -1;

			return duplicate (*target, source);
		}

		int duplicate (Cell *target, Cell const& source) const
		{
			if (target == nullptr)
				return -1;

			return duplicate (*target, source);
		}

		// Upstream call doesn't take ncplane* but we put it here for parity with has_no_background below
		bool has_no_foreground (Cell &cell) const noexcept
		{
			return cell.has_no_foreground ();
		}

		bool has_no_background (Cell &cell) const noexcept
		{
			return cell_nobackground_p (plane, cell);
		}

		const char* get_extended_gcluster (Cell &cell) const noexcept
		{
			return cell_extended_gcluster (plane, cell);
		}

		const char* get_extended_gcluster (Cell const& cell) const noexcept
		{
			return cell_extended_gcluster (plane, cell);
		}

		static Plane* map_plane (ncplane *ncp, Plane *associated_plane = nullptr) noexcept;

	protected:
		explicit Plane (ncplane *plane, bool is_stdplane) noexcept
			: plane (plane),
			  is_stdplane (is_stdplane)
		{}

		static void unmap_plane (Plane *p) noexcept;

	private:
		ncplane* create_plane (Plane &n, int rows, int cols, int yoff, NCAlign align, void *opaque) noexcept
		{
			return ncplane_aligned (
				n.plane,
				rows,
				cols,
				yoff,
				static_cast<ncalign_e>(align),
				opaque
			);

			map_plane (plane, this);
		}

	private:
		ncplane *plane;
		bool is_stdplane = false;
		static std::map<ncplane*,Plane*> *plane_map;
		static std::mutex plane_map_mutex;

		friend class NotCurses;
		friend class Visual;
		friend class PanelReel;
		friend class Tablet;
	};
}
#endif
