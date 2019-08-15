#ifndef TEST_BUILD

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstdint>
#include "shell/command-line-parser.h"
#include "imaging/bitmap.h"
#include "imaging/bmp-format.h"
#include "midi/midi.h"
using namespace midi;
using namespace std;
using namespace shell;
using namespace imaging;

void draw_rectangle(Bitmap& bitmap, const Position& pos, const uint32_t& width, const uint32_t& height, const Color& color){
	for (uint32_t i = 0; i < width; i++){
		for (uint32_t j = 0; j < height; j++){
			bitmap[Position(pos.x + i, pos.y + j)] = color;
		}
	}
}

int getWidth(vector<NOTE> notes)
{
	int res = 0;
	for (NOTE n : notes){
		if (value(n.start + n.duration) > res){
			res = value(n.start + n.duration);
		}
	}
	/*cout << "Width with scale 1 =========== " << res << endl;*/
	return res;
}

int getMinNote(vector<NOTE> notes){
	int res = 128;
	for (NOTE n : notes){
		if (value(n.note_number) < res){
			res = value(n.note_number);
		}
	}
	//cout << "Lowest note found ====== " << res << endl;
	return res;
}

int getMaxNote(vector<NOTE> notes){
	int res = 0;
	for (NOTE n : notes){
		if (value(n.note_number) > res){
			res = value(n.note_number);
		}
	}
	//cout << "highest note found ====== " << res << endl;
	return res;
}

int main(int argn, char* argv[]){
	Bitmap bp(600, 600);
	draw_rectangle(bp, Position(0, 100), 600, 400, Color(1,1,0));
	save_as_bmp("C:\\tmp\\bitmaptester.bmp", bp);

	bp = *bp.slice(0, 100, 600, 400);
	save_as_bmp("C:\\tmp\\bitmaptester1.bmp", bp);

	string filename = "C:\\tmp\\darude-sandstorm.mid";
	string outfile = "C:\\tmp\\frames\\frame%d.bmp";
	uint32_t height = 16;
	uint32_t scale = 10;
	uint32_t step = 100;
	uint32_t framewidth = 1000;



	//make parser
	CommandLineParser parser;
	parser.add_argument(string("-w"), &framewidth);
	parser.add_argument(string("-d"), &step);
	parser.add_argument(string("-s"), &scale);
	parser.add_argument(string("-h"), &height);
	parser.process(vector<string>(argv + 1, argv + argn));
	vector<string> arrgs = parser.positional_arguments();

	if (arrgs.size() >= 1){
		filename = arrgs[0];
	}
	if (arrgs.size() >= 2){
		outfile = arrgs[1];
	}

	ifstream in(filename, ifstream::binary);

	vector<NOTE> notes = read_notes(in);

	uint32_t mapwidth = getWidth(notes) / scale;

	if (framewidth == 0){
		framewidth = mapwidth;
	}

	int low = getMinNote(notes);
	int high = getMaxNote(notes);

	cout <<  "GOING IN FOR THE NOTES " << endl;

	Bitmap bitmap(mapwidth, (127) * height );
	for (int i = 0; i <= 127; i++) {
		for (NOTE n : notes) {
			if (n.note_number == NoteNumber(i)) {
				auto posX = value(n.start) / scale;
				auto posY = (127 - i) * height;
				auto widthBM = value(n.duration) / scale;
				int g = i;
				double colourBlue = (1.0 / static_cast<double>(g/height) );
			
				draw_rectangle(bitmap, Position(posX,posY), widthBM, height, Color(1, 0, colourBlue));
			}
		}
	}
	cout << "Notes have been read in" << endl;

	//lelijk zwart ding weg doen 
	auto newHeightOfBitMap = (high - low + 1) * height;
	bitmap = *bitmap.slice(0, height * (127 - high),mapwidth, newHeightOfBitMap);

	//mooie frames maken 
	for (int i = 0; i <= mapwidth - framewidth; i += step)
	{
		
		Bitmap temp = *bitmap.slice(i, 0, framewidth,newHeightOfBitMap).get();

		stringstream teller;
		teller << setfill('0') << setw(5) << (i / step);
		string out = outfile; save_as_bmp(out.replace(out.find("%d"), 2, teller.str()), temp);

		cout << "Frame: " << (i / step) << " created" << endl;
	}
	//video maken 
	/*system("ffmpeg -i frame%05d.bmp -c:v libx264 -r 30 -pix_fmt yuv420p movie.mp4");*/
}

#endif
