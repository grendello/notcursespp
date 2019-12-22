#ifndef __NCPP_TABLET_HH
#define __NCPP_TABLET_HH

#include <map>
#include <mutex>

#include <notcurses.h>

#include "Root.hh"

namespace ncpp
{
	class Plane;

	class Tablet : public Root
	{
	public:
		explicit Tablet (tablet *t)
			: _tablet (t)
		{};

		void* get_userptr () const noexcept
		{
			return tablet_userptr (_tablet);
		}

		Plane* get_plane () const noexcept;

	protected:
		static Tablet* map_tablet (tablet *ncp) noexcept;
		static void unmap_tablet (Tablet *p) noexcept;

		tablet* get_tablet () const noexcept
		{
			return _tablet;
		}

	private:
		tablet *_tablet;
		static std::map<tablet*,Tablet*> *tablet_map;
		static std::mutex tablet_map_mutex;

		friend class PanelReel;
	};
}
#endif
