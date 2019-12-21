#ifndef __NCPP_VISUAL_HH
#define __NCPP_VISUAL_HH

#include <notcurses.h>

#include "Root.hh"
#include "NCScale.hh"

namespace ncpp
{
	class Plane;

	class NCPP_API_EXPORT Visual : public Root
	{
	protected:
		explicit Visual (ncplane *plane, const char *file, int *averr) noexcept
		{
			visual = ncplane_visual_open (plane, file, averr);
		}

		explicit Visual (const char *file, int *averr, int y, int x, NCScale scale) noexcept
		{
			visual = ncvisual_open_plane (get_notcurses (), file, averr, y, x, static_cast<ncscale_e>(scale));
		}

	public:
		~Visual () noexcept
		{
			if (visual == nullptr)
				return;

			ncvisual_destroy (visual);
		}

		operator bool () noexcept
		{
			return visual != nullptr;
		}

		operator ncvisual* () const noexcept
		{
			return visual;
		}

		operator ncvisual const* () const noexcept
		{
			return visual;
		}

		static Visual* open_plane (const char *file, int *averr, int y, int x, NCScale scale) noexcept
		{
			return new Visual (file, averr, y, x, scale);
		}

		AVFrame* decode (int *averr) const noexcept
		{
			return ncvisual_decode (visual, averr);
		}

		bool render (int begy, int begx, int leny, int lenx) const noexcept
		{
			return ncvisual_render (visual, begy, begx, leny, lenx) != -1;
		}

		int stream (int *averr, streamcb streamer) const noexcept
		{
			return ncvisual_stream (get_notcurses (), visual, averr, streamer);
		}

		Plane* get_plane () const noexcept;

	private:
		ncvisual *visual;

		friend class Plane;
	};
}
#endif
