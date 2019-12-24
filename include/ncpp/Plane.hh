#ifndef __NCPP_PLANE_HH
#define __NCPP_PLANE_HH

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
			plane = notcurses_newplane (
				get_notcurses (),
				rows,
				cols,
				yoff,
				xoff,
				opaque
			);

			map_plane (plane, this);
		}

		explicit Plane (ncplane *plane) noexcept
			: plane (plane)
		{}

		~Plane () noexcept
		{
			if (plane == nullptr)
				return;

			ncplane_destroy (plane);
			unmap_plane (this);
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
			) == 0;
		}

		bool fadeout (timespec *ts) const noexcept
		{
			return ncplane_fadeout (plane, ts) != -1;
		}

		void erase () const noexcept
		{
			ncplane_erase (plane);
		}

		void get_dim (int *rows, int *cols) const noexcept
		{
			ncplane_dim_yx (plane, rows, cols);
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

		bool move_above (Plane &above) const noexcept
		{
			return ncplane_move_above (plane, above.plane) != -1;
		}

		bool cursor_move_yx (int y, int x) const noexcept
		{
			return ncplane_cursor_move_yx (plane, y, x) != -1;
		}

		void get_cursor_yx (int *y, int *x) const noexcept
		{
			ncplane_cursor_yx (plane, y, x);
		}

		int putc (const Cell &c) const noexcept
		{
			return ncplane_putc (plane, c);
		}

		int putc (int y, int x, const Cell &c) const noexcept
		{
			return ncplane_putc_yx (plane, y, x, c);
		}

		int putc (char c) const noexcept
		{
			return ncplane_putsimple (plane, c);
		}

		int putc (int y, int x, char c) const noexcept
		{
			return ncplane_putsimple_yx (plane, y, x, c);
		}

		int putc (const char *gclust, uint32_t attr, uint64_t channels, int *sbytes) const noexcept
		{
			return ncplane_putegc (plane, gclust, attr, channels, sbytes);
		}

		int putc (int y, int x, const char *gclust, uint32_t attr, uint64_t channels, int *sbytes) const noexcept
		{
			return ncplane_putegc_yx (plane, y, x, gclust, attr, channels, sbytes);
		}

		int putc (const wchar_t *gclust, uint32_t attr, uint64_t channels, int *sbytes) const noexcept
		{
			return ncplane_putwegc (plane, gclust, attr, channels, sbytes);
		}

		int putc (int y, int x, const wchar_t *gclust, uint32_t attr, uint64_t channels, int *sbytes) const noexcept
		{
			return ncplane_putwegc_yx (plane, y, x, gclust, attr, channels, sbytes);
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

		bool printf (const char* format, ...) const noexcept
			__attribute__ ((format (printf, 2, 3)))
		{
			va_list va;

			va_start (va, format);
			int ret = ncplane_vprintf (plane, format, va);
			va_end (va);

			return ret >= 0;
		}

		bool printf (int y, int x, const char *format, ...) const noexcept
			__attribute__ ((format (printf, 4, 5)))
		{
			va_list va;

			va_start (va, format);
			int ret = ncplane_vprintf_yx (plane, y, x, format, va);
			va_end (va);

			return ret >= 0;
		}

		bool printf (int y, NCAlign align, const char *format, ...) const noexcept
			__attribute__ ((format (printf, 4, 5)))
		{
			va_list va;

			va_start (va, format);
			bool ret = vprintf (y, align, format, va);
			va_end (va);

			return ret;
		}

		bool vprintf (const char* format, va_list ap) const noexcept
		{
			return ncplane_vprintf (plane, format, ap) >= 0;
		}

		bool vprintf (int y, int x, const char* format, va_list ap) const noexcept
		{
			return ncplane_vprintf_yx (plane, y, x, format, ap) >= 0;
		}

		bool vprintf (int y, NCAlign align, const char *format, va_list ap) const noexcept
		{
			return ncplane_vprintf_aligned (plane, y, static_cast<ncalign_e>(align), format, ap) >= 0;
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
			return ncplane_get_channels (plane);
		}

		uint32_t get_attr () const noexcept
		{
			return ncplane_get_attr (plane);
		}

		unsigned get_bchannel () const noexcept
		{
			return ncplane_get_bchannel (plane);
		}

		unsigned get_fchannel () const noexcept
		{
			return ncplane_get_fchannel (plane);
		}

		unsigned get_fg () const noexcept
		{
			return ncplane_get_fg (plane);
		}

		unsigned get_bg () const noexcept
		{
			return ncplane_get_bg (plane);
		}

		unsigned get_fg_alpha () const noexcept
		{
			return ncplane_get_fg_alpha (plane);
		}

		bool set_fg_alpha (int alpha) const noexcept
		{
			return ncplane_set_fg_alpha (plane, alpha) != -1;
		}

		unsigned get_bg_alpha () const noexcept
		{
			return ncplane_get_bg_alpha (plane);
		}

		bool set_bg_alpha (int alpha) const noexcept
		{
			return ncplane_set_bg_alpha (plane, alpha) != -1;
		}

		unsigned get_fg_rgb (unsigned *r, unsigned *g, unsigned *b) const noexcept
		{
			return ncplane_get_fg_rgb (plane, r, g, b);
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
			return ncplane_get_bg_rgb (plane, r, g, b);
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

		bool set_default (Cell &c) const noexcept
		{
			return ncplane_set_default (plane, c) >= 0;
		}

		bool get_default (Cell &c) const noexcept
		{
			return ncplane_default (plane, c) >= 0;
		}

		bool at_cursor (Cell &c) const noexcept
		{
			return ncplane_at_cursor (plane, c) >= 0;
		}

		bool at_yx (int y, int x, Cell &c) const noexcept
		{
			return ncplane_at_yx (plane, y, x, c) >= 0;
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

		const char* get_extended_gcluster (Cell &cell) const noexcept
		{
			return cell_extended_gcluster (plane, cell);
		}

	protected:
		static Plane* map_plane (ncplane *ncp, Plane *associated_plane = nullptr) noexcept;
		static void unmap_plane (Plane *p) noexcept;

	private:
		ncplane *plane;
		static std::map<ncplane*,Plane*> *plane_map;
		static std::mutex plane_map_mutex;

		friend class NotCurses;
		friend class Visual;
		friend class PanelReel;
		friend class Tablet;
	};
}
#endif
