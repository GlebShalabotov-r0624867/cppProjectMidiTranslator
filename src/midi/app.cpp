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
	int res = 127;
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
	draw_rectangle(bp, Position(100, 100), 300, 300, Color(1,1,0));
	save_as_bmp("C:\\tmp\\bitmaptester.bmp", bp);


}

#endif
