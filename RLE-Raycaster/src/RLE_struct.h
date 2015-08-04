#pragma once

#include "core.h"
/*
#include "compression/huffman.h"
#include "compression/lz.h"
#include "compression/rice.h"
#include "compression/rle.h"
#include "compression/shannonfano.h"
*/

struct RLE2
{
	struct Stick
	{
		short start;
		short end;
	};
	struct StickCol
	{

		unsigned int mat1;
		unsigned int mat2;

	};

	struct Elem //X-Z-Element
	{
		unsigned int start24_len8; 
	};

	Elem *map;
	Stick *sticks;
	int numsticks;	

	struct MipMap
	{
		int sx,sy,sz;
		Elem *map;
		Stick *sticks;
		StickCol *sticks_col ;
		int numsticks;	
	};

	int nummipmaps;

	MipMap *mipmap;
	MipMap *mipmap_coarse;

	int mipmap_coarse_scale;
	int mipmap_coarse_shift;
	int sx,sy,sz;

	/*------------------------------------------------------*/
};


