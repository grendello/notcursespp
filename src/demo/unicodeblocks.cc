#include <config.hh>

#include <memory>
#include <cerrno>
#include <cwchar>
#include <cwctype>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <ncpp/NotCurses.hh>

#include "demo.hh"

using namespace ncpp;

// show unicode blocks. a block is always a multiple of 16 codepoints.
constexpr int BLOCKSIZE = 512; // show this many per page
constexpr int CHUNKSIZE = 32;  // show this many per line

static bool
fade_block (std::shared_ptr<Plane> nn, const struct timespec* subdelay)
{
	bool ret = nn->fadein (subdelay, demo_fader);
	nn.reset ();

	return ret;
}

static bool
draw_block (std::shared_ptr<Plane> nn, uint32_t blockstart)
{
	Cell ul, ur;
	Cell ll, lr;
	Cell hl, vl;

	nn->load_rounded_box (0, 0, ul, ur, ll, lr, hl, vl);
	ul.set_bg_alpha (Cell::AlphaTransparent);
	ur.set_bg_alpha (Cell::AlphaTransparent);
	ll.set_bg_alpha (Cell::AlphaTransparent);
	lr.set_bg_alpha (Cell::AlphaTransparent);
	ll.set_fg_rgb (255, 255, 255);
	lr.set_fg_rgb (255, 255, 255);
	ul.set_fg_rgb (255, 255, 255);
	ur.set_fg_rgb (255, 255, 255);
	hl.set_fg_rgb (255, 255, 255);
	vl.set_fg_rgb (255, 255, 255);
	hl.set_bg_rgb (0, 0, 0);
	vl.set_bg_rgb (0, 0, 0);

	int dimx, dimy;
	nn->get_dim (&dimy, &dimx);
	if (!nn->box_sized (ul, ur, ll, lr, hl, vl, dimy, dimx, 0)) {
		return -false;
	}
	nn->release (ul); nn->release (ur); nn->release (hl);
	nn->release (ll); nn->release (lr); nn->release (vl);

	int chunk;
	for (chunk = 0 ; chunk < BLOCKSIZE / CHUNKSIZE ; ++chunk) {
		int z;
		// 16 to a line
		auto utf8arr = new char[MB_CUR_MAX * 2 + 1];
		for (z = 0 ; z < CHUNKSIZE ; ++z) {
			wchar_t w[3] = { static_cast<wchar_t>(blockstart) + chunk * CHUNKSIZE + z, L'\u200e', L'\0' };

			if (wcswidth (w, 3) >= 1 && iswprint (w[0])) {
				mbstate_t ps;
				memset (&ps, 0, sizeof(ps));

				const wchar_t *wptr = w;
				int bwc = wcsrtombs (utf8arr, &wptr, sizeof(utf8arr), &ps);
				if (bwc < 0) {
					uint32_t ch = blockstart + chunk * CHUNKSIZE + z;
					std::cerr << "Couldn't convert " << ch << " (" << std::hex << ch << std::dec << ") (" << w[0] << ") (" << strerror (errno) << ")" << std::endl;
					return false;
				}
				if (wcwidth(w[0]) < 2) {
					utf8arr[bwc++] = ' ';
				}
				utf8arr[bwc++] = '\0';
			} else { // don't dump non-printing codepoints
				strcpy (utf8arr, "  ");
			}
			nn->set_fg_rgb (0xad + z * 2, 0xff, 0x2f - z * 2);
			nn->set_bg_rgb (8 * chunk, 8 * chunk + z, 8 * chunk);

			if (nn->putstr (chunk + 1, z * 2 + 1, utf8arr) < 0) {
				return false;
			}
		}
		delete[] utf8arr;
	}

	return true;
}

bool unicodeblocks_demo (NotCurses &nc)
{
	std::unique_ptr<Plane> n (nc.get_stdplane ());

	int maxx, maxy;
	nc.get_term_dim (&maxy, &maxx);

	// some blocks are good for the printing, some less so. some are only
	// marginally covered by mainstream fonts, some not at all. we explicitly
	// list the ones we want.
	const struct {
		const char* name;
		uint32_t start;
	} blocks[] = {
		{ /*.name =*/ "Basic Latin, Latin 1 Supplement, Latin Extended", /*.start =*/ 0, },
		{ /*.name =*/ "IPA Extensions, Spacing Modifiers, Greek and Coptic", /*.start =*/ 0x200, },
		{ /*.name =*/ "Cyrillic, Cyrillic Supplement, Armenian, Hebrew", /*.start =*/ 0x400, },
		{ /*.name =*/ "Arabic, Syriac, Arabic Supplement", /*.start =*/ 0x600, },
		{ /*.name =*/ "Samaritan, Mandaic, Devanagari, Bengali", /*.start =*/ 0x800, },
		{ /*.name =*/ "Gurmukhi, Gujarati, Oriya, Tamil", /*.start =*/ 0xa00, },
		{ /*.name =*/ "Telugu, Kannada, Malayalam, Sinhala", /*.start =*/ 0xc00, },
		{ /*.name =*/ "Thai, Lao, Tibetan", /*.start =*/ 0xe00, },
		{ /*.name =*/ "Myanmar, Georgian, Hangul Jamo", /*.start =*/ 0x1000, },
		{ /*.name =*/ "Ethiopic, Ethiopic Supplement, Cherokee", /*.start =*/ 0x1200, },
		{ /*.name =*/ "Canadian", /*.start =*/ 0x1400, },
		{ /*.name =*/ "Runic, Tagalog, Hanunoo, Buhid, Tagbanwa, Khmer", /*.start =*/ 0x1600, },
		{ /*.name =*/ "Mongolian, Canadian Extended, Limbu, Tai Le", /*.start =*/ 0x1800, },
		{ /*.name =*/ "Buginese, Tai Tham, Balinese, Sundanese, Batak", /*.start =*/ 0x1a00, },
		{ /*.name =*/ "Lepcha, Ol Chiki, Vedic Extensions, Phonetic Extensions", /*.start =*/ 0x1c00, },
		{ /*.name =*/ "Latin Extended Additional, Greek Extended", /*.start =*/ 0x1e00, },
		{ /*.name =*/ "General Punctuation, Letterlike Symbols, Arrows", /*.start =*/ 0x2000, },
		{ /*.name =*/ "Mathematical Operators, Miscellaneous Technical", /*.start =*/ 0x2200, },
		{ /*.name =*/ "Control Pictures, Box Drawing, Block Elements", /*.start =*/ 0x2400, },
		{ /*.name =*/ "Miscellaneous Symbols, Dingbats", /*.start =*/ 0x2600, },
		{ /*.name =*/ "Braille Patterns, Supplemental Arrows", /*.start =*/ 0x2800, },
		{ /*.name =*/ "Supplemental Mathematical Operators", /*.start =*/ 0x2a00, },
		{ /*.name =*/ "Glagolitic, Georgian Supplement, Tifinagh", /*.start =*/ 0x2c00, },
		{ /*.name =*/ "Supplemental Punctuation, CJK Radicals", /*.start =*/ 0x2e00, },
		{ /*.name =*/ "CJK Symbols and Punctuation", /*.start =*/ 0x3000, },
		{ /*.name =*/ "Enclosed CJK Letters and Months", /*.start =*/ 0x3200, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A", /*.start =*/ 0x3400, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x3600, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x3800, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x3a00, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x3c00, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x3e00, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4000, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4200, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4400, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4600, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4800, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A (cont.)", /*.start =*/ 0x4a00, },
		{ /*.name =*/ "CJK Unified Ideographs Extension A, Yijang Hexagram", /*.start =*/ 0x4c00, },
		{ /*.name =*/ "CJK Unified Ideographs", /*.start =*/ 0x4e00, },
		{ /*.name =*/ "Yi Syllables", /*.start =*/ 0xa000, },
		{ /*.name =*/ "Yi Syllables", /*.start =*/ 0xa200, },
		{ /*.name =*/ "Yi Syllables, Yi Radicals, Lisu, Vai", /*.start =*/ 0xa400, },
		{ /*.name =*/ "Vai, Cyrillic Extended-B, Bamum, Tone Letters, Latin Extended-D", /*.start =*/ 0xa600, },
		{ /*.name =*/ "Halfwidth and Fullwidth Forms", /*.start =*/ 0xff00, },
		{ /*.name =*/ "Linear B Syllabary, Linear B Ideograms, Aegean Numbers, Phaistos Disc", /*.start =*/ 0x10000, },
		{ /*.name =*/ "Lycian, Carian, Coptic Epact Numbers, Old Italic, Gothic, Old Permic", /*.start =*/ 0x10200, },
		{ /*.name =*/ "Cuneiform", /*.start =*/ 0x12000, },
		{ /*.name =*/ "Cuneiform (cont.)", /*.start =*/ 0x12200, },
		{ /*.name =*/ "Byzantine Musical Symbols, Musical Symbols", /*.start =*/ 0x1d000, },
		{ /*.name =*/ "Ancient Greek Musical Notation, Mayan Numerals, Tai Xuan Jing, Counting Rods", /*.start =*/ 0x1d200, },
		{ /*.name =*/ "Mathematical Alphanumeric Symbols", /*.start =*/ 0x1d400, },
		{ /*.name =*/ "Mathematical Alphanumeric Symbols (cont.)", /*.start =*/ 0x1d600, },
		{ /*.name =*/ "Sutton SignWriting", /*.start =*/ 0x1d800, },
		{ /*.name =*/ "Glagolitic Supplement, Nyiakeng Puachue Hmong", /*.start =*/ 0x1e000, },
		{ /*.name =*/ "Ottoman Siyaq Numbers", /*.start =*/ 0x1ed00, },
		{ /*.name =*/ "Arabic Mathematical Alphabetic Symbols", /*.start =*/ 0x1ee00, },
		{ /*.name =*/ "Mahjong Tiles, Domino Tiles, Playing Cards", /*.start =*/ 0x1f000, },
		{ /*.name =*/ "Enclosed Ideographic Supplement, Miscellaneous Symbols", /*.start =*/ 0x1f200, },
		{ /*.name =*/ "Miscellaneous Symbols and Pictographs (cont.)", /*.start =*/ 0x1f400, },
		{ /*.name =*/ "Emoticons, Ornamental Dingbats, Transport and Map Symbols", /*.start =*/ 0x1f600, },
		{ /*.name =*/ "Supplemental Arrows-C, Supplemental Symbols", /*.start =*/ 0x1f800, },
		{ /*.name =*/ "Chess Symbols, Symbols and Pictographs Extended-A", /*.start =*/ 0x1fa00, },
	};

	size_t sindex;
	// we don't want a full delay period for each one, urk...or do we?
	struct timespec subdelay;
	uint64_t nstotal = timespec_to_ns (&demodelay);
	ns_to_timespec (nstotal / 3, &subdelay);

	for (sindex = 0 ; sindex < sizeof(blocks) / sizeof(*blocks) ; ++sindex) {
		n->set_bg_rgb (0, 0, 0);

		uint32_t blockstart = blocks[sindex].start;
		const char* description = blocks[sindex].name;
		n->set_fg_rgb (0xad, 0xd8, 0xe6);
		if (n->printf (1, NCAlign::Center, "Unicode points %05x–%05x", blockstart, blockstart + BLOCKSIZE) <= 0) {
			return false;
		}

		int xstart = (maxx - ((CHUNKSIZE * 2) + 3)) / 2;
		auto nn = std::make_shared<Plane>(BLOCKSIZE / CHUNKSIZE + 2, (CHUNKSIZE * 2) + 2, 3, xstart);
		if (nn == nullptr || !*nn) {
			return false;
		}

		if (hud != nullptr) {
			nn->move_below_unsafe (hud);
		}

		if (!draw_block (nn, blockstart)) {
			return false;
		}

		n->set_fg_rgb (0x40, 0xc0, 0x40);
		if (!n->cursor_move (6 + BLOCKSIZE / CHUNKSIZE, 0)) {
			return false;
		}

		if(n->printf ("%*.*s", maxx, maxx, "") <= 0) {
			return false;
		}

		if(n->printf (6 + BLOCKSIZE / CHUNKSIZE, NCAlign::Center, "%s", description) <= 0) {
			return false;
		}

		if (!fade_block (nn, &subdelay)) { // destroys nn
			return false;
		}

		// for a 32-bit wchar_t, we would want up through 24 bits of block ID. but
		// really, the vast majority of space is unused.
		blockstart += BLOCKSIZE;
	}

	return true;
}
