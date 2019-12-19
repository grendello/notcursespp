#ifndef __NCPP_VISUAL_HH
#define __NCPP_VISUAL_HH

#include <notcurses.h>

#include "Root.hh"

namespace ncpp
{
	class NCPP_API_EXPORT Visual : public Root
	{
	public:
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

		static Visual* open_plane (const char *file, int *averr, int y, int x, bool stretch) noexcept
		{
			return new Visual (file, averr, y, x, stretch);
		}

		bool render () const noexcept
		{
			return ncvisual_render (visual) != -1;
		}

		int stream (int *averr, streamcb streamer) const noexcept
		{
			return ncvisual_stream (get_notcurses (), visual, averr, streamer);
		}

	protected:
		explicit Visual (ncplane *plane, const char *file, int *averr) noexcept
		{
			visual = ncplane_visual_open (plane, file, averr);
		}

		explicit Visual (const char *file, int *averr, int y, int x, bool stretch) noexcept
		{
			visual = ncvisual_open_plane (get_notcurses (), file, averr, y, x, stretch);
		}

		~Visual () noexcept
		{
			if (visual == nullptr)
				return;

			ncvisual_destroy (visual);
		}

	private:
		ncvisual *visual;

		friend class Plane;
	};
}
#endif
