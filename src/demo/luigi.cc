#include <config.hh>

#include <memory>
#include <ncpp/NotCurses.hh>

#include "demo.hh"

using namespace ncpp;

//0 = trans
//1 = white
//2 = yellow
//3 = green

static const char* luigis[] = {
	"0000000000000000"
	"0000000000000000"
	"0000000111110000"
	"0000011111120000"
	"0000111111220000"
	"0000111111111110"
	"0000333223222000"
	"0003223223322220"
	"0003223322222222"
	"0033223322232222"
	"0033222223333330"
	"0003322222333330"
	"0000032222222200"
	"0000311122200000"
	"0003133313000000"
	"0003133331300000"
	"0033133333112200"
	"0031133333332222"
	"0031113333332222"
	"0001113333333222"
	"0001111333333222"
	"0001111113331000"
	"0001111111111000"
	"0001111111113000"
	"3333111111131100"
	"3333111113311100"
	"3333111131111000"
	"3333111001111000"
	"3333000003333000"
	"3300000003333000"
	"3000000003333330"
	"0000000003333330",

	"0000000000000000"
	"0000001111100000"
	"0000111111200000"
	"0001111112200000"
	"0001111111111100"
	"0003332232220000"
	"0032232233222200"
	"0032233222222220"
	"0332233222322220"
	"0332222233333300"
	"0033222223333300"
	"0003322222222000"
	"0000111122000000"
	"0003133113300000"
	"0031333311300000"
	"0031333311330000"
	"0031333311130000"
	"0031333332230000"
	"0031333322220000"
	"0011133322221000"
	"0011133322221100"
	"0011113322211100"
	"0011111133111100"
	"0001111133311000"
	"0000111333333000"
	"0000113333330000"
	"0000011333300000"
	"0000031113330000"
	"0000033330330000"
	"0000333330000000"
	"0000333333300000"
	"0000003333300000",

	"0000001111100000"
	"0000111111200000"
	"0001111112200000"
	"0001111111111100"
	"0003332232220000"
	"0032232233222200"
	"0032233222222220"
	"0332233222322220"
	"0332222233333300"
	"0333222223333300"
	"0003322222222000"
	"0000033322000000"
	"0000111133100020"
	"0003333113310222"
	"0033333311313222"
	"0333333311331222"
	"0333333311331323"
	"0333333111331330"
	"3333331112132300"
	"3333111111111000"
	"2222211111111000"
	"2222211111111003"
	"2222111111111033"
	"0222111111133333"
	"0001311111133333"
	"0031131111133333"
	"3331113311133333"
	"3333111100033333"
	"3333310000000000"
	"0333000000000000"
	"0333000000000000"
	"0033300000000000",

	NULL
};

static bool
draw_luigi (std::shared_ptr<Plane> n, const char* sprite)
{
	Cell bgc;
	bgc.set_fg_alpha (Cell::AlphaTransparent);
	bgc.set_bg_alpha (Cell::AlphaTransparent);
	n->set_base (bgc);
	n->release (bgc);

	size_t s;
	int sbytes;
	uint64_t channels = 0;
	// optimization so we can elide more color changes, see README's "#perf"
	channels_set_bg_rgb (&channels, 0x00, 0x00, 0x00);
	for (s = 0 ; sprite[s] ; ++s) {
		switch (sprite[s]) {
			case '0':
				n->cursor_move ((s + 1) / 16, (s + 1) % 16);
				break;

			case '1':
				channels_set_fg_rgb (&channels, 0xff, 0xff, 0xff);
				break;

			case '2':
				channels_set_fg_rgb (&channels, 0xe3, 0x9d, 0x25);
				break;

			case '3':
				channels_set_fg_rgb (&channels, 0x3a, 0x84, 0x00);
				break;
		}

		if (sprite[s] != '0') {
			if (n->putc ("\u2588", 0, channels, &sbytes) != 1) {
				return false;
			}
		}
	}

	return true;
}

bool luigi_demo (NotCurses &nc)
{
	Plane *n = nc.get_stdplane ();
	int averr = 0;
	char *map = find_data ("megaman2.bmp");

	std::unique_ptr<Visual> nv (n->visual_open (map, &averr));
	delete map;

	if (nv == nullptr) {
		return false;
	}

	if(nv->decode (&averr) == nullptr) {
		return false;
	}

	if(!nv->render (0, 0, 0, 0)) {
		return false;
	}

	int rows, cols;
	n->get_dim (&rows, &cols);

	// he should be walking on the platform ~4/5 of the way down
	const int height = 32;
	int yoff = rows * 4 / 5 - height + 1; // tuned
	std::shared_ptr<Plane> lns[3];
	int i;
	std::shared_ptr<Plane> &lastseen = lns[0];
	for (i = 0 ; i < 3 ; ++i) {
		lns[i] = std::make_shared<Plane> (height, 16, yoff, 0);
		if (lns[i] == nullptr) {
			return false;
		}

		lastseen = lns[i];
		draw_luigi (lns[i], luigis[i]);
		lastseen->move_bottom (); // all start hidden underneath stdplane
	}

	struct timespec stepdelay;
	ns_to_timespec (timespec_to_ns (&demodelay) / (cols - 16 - 1), &stepdelay);

	for (i = 0 ; i < cols - 16 - 1 ; ++i) {
		if (i + 16 >= cols - 16 - 1) {
			--yoff;
		} else {
			lastseen->move_bottom (); // hide the previous sprite
			lastseen = lns[i % 3];
			lastseen->move_top ();
		}
		lastseen->move (yoff, i);
		demo_render (nc);
		nanosleep (&stepdelay, nullptr);
	}

	return true;
}
