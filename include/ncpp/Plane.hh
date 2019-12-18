#ifndef __NCPP_PLANE_HH
#define __NCPP_PLANE_HH

#include <map>
#include <mutex>
#include <notcurses.h>

#include "Root.hh"
#include "Cell.hh"
#include "CellStyle.hh"

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
			return ncplane_move_yx (plane, y, x) == 0;
		}

		bool move_top () const noexcept
		{
			return ncplane_move_top (plane) == 0;
		}

		bool move_bottom () const noexcept
		{
			return ncplane_move_bottom (plane) == 0;
		}

		bool move_below (Plane &below) const noexcept
		{
			return ncplane_move_below (plane, below.plane) == 0;
		}

		bool move_above (Plane &above) const noexcept
		{
			return ncplane_move_above (plane, above.plane) == 0;
		}

		bool cursor_move_yx (int y, int x) const noexcept
		{
			return ncplane_cursor_move_yx (plane, y, x) == 0;
		}

		void get_cursor_yx (int *y, int *x) const noexcept
		{
			ncplane_cursor_yx (plane, y, x);
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
			return ncplane_set_default (plane, c) == 0;
		}

		bool get_default (Cell &c) const noexcept
		{
			return ncplane_default (plane, c) == 0;
		}

		bool at_cursor (Cell &c) const noexcept
		{
			return ncplane_at_cursor (plane, c) == 0;
		}

		bool at_yx (int y, int x, Cell &c) const noexcept
		{
			return ncplane_at_yx (plane, y, x, c) == 0;
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

	protected:
		static Plane* map_plane (ncplane *ncp, Plane *associated_plane = nullptr) noexcept;
		static void unmap_plane (Plane *p) noexcept;

	private:
		ncplane *plane;
		static std::map<ncplane*,Plane*> *plane_map;
		static std::mutex plane_map_mutex;

		friend class NotCurses;
	};
}
#endif
