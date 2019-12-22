#include <cstdlib>
#include <clocale>

#include <ncpp/NotCurses.hh>
#include <ncpp/PanelReel.hh>
#include <ncpp/NCKey.hh>

using namespace ncpp;

// FIXME ought be able to get pr from tablet, methinks?
static PanelReel* PR;

class TabletCtx {
public:
    TabletCtx () : lines (rand() % 5 + 3)
	{}

    int getLines () const
	{
		return lines;
    }

private:
    int lines;
};

int tabletfxn (tablet* tb, int begx, int begy, int maxx, int maxy, bool cliptop)
{
	Tablet *t = Tablet::get_instance (tb); // TODO: ugly, replace with something better
	Plane* p = t->get_plane ();
	auto *tctx = t->get_userptr<TabletCtx> ();
	p->erase ();
	Cell c (' ');
	c.set_bg ((((uintptr_t)t) % 0x1000000) + cliptop + begx + maxx);
	p->set_default (c);
	p->release (c);

	return tctx->getLines () > maxy - begy ? maxy - begy : tctx->getLines ();
}

int main (void)
{
	if (setlocale(LC_ALL, "") == nullptr) {
		return EXIT_FAILURE;
	}

	NotCurses &nc = NotCurses::get_instance ();
	nc.init ();
	if(!nc) {
		return EXIT_FAILURE;
	}

	Plane* nstd = nc.get_stdplane ();
	int dimy, dimx;
	nstd->get_dim (&dimy, &dimx);

	auto n = new Plane (dimy - 1, dimx, 1, 0);
	if(!n) {
		return EXIT_FAILURE;
	}

	if (!n->set_fg (0xb11bb1)) {
		return EXIT_FAILURE;
	}

	if(n->putstr (0, "(a)dd (d)el (q)uit", NCAlign::Center) <= 0) {
		return EXIT_FAILURE;
	}

	// TODO: add a channels class
	channels_set_fg (&PanelReel::default_options.focusedchan, 0xffffff);
	channels_set_bg (&PanelReel::default_options.focusedchan, 0x00c080);

	PanelReel* pr = n->panelreel_create ();
	if (pr == nullptr || !nc.render ()) {
		return EXIT_FAILURE;
	}

	PR = pr; // FIXME eliminate
	char32_t key;
	while ((key = nc.getc (true)) != (char32_t)-1) {
		switch (key) {
			case 'q':
				return EXIT_SUCCESS;

			case 'a':{
				TabletCtx* tctx = new TabletCtx ();
				pr->add (nullptr, nullptr, tabletfxn, tctx);
				break;
			}
			case 'd':
				pr->del_focused ();
				break;

			case static_cast<char32_t>(NCKey::Up): // TODO: fix, ugly
				pr->prev ();
				break;

			case static_cast<char32_t>(NCKey::Down): // TODO: fix, ugly
				pr->next ();
				break;

			default:
				break;
		}
		if (!nc.render ()) {
			break;
		}
	}

	return EXIT_FAILURE;
}
