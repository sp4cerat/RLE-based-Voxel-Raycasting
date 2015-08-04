#pragma once
#include "Core.h"

extern "C" void create_cuda_1d_texture(char* data32, int size);
extern "C" void create_cuda_2d_texture(uint* data64, int width,int height);

struct Map4
{
	int    sx,sy,sz,slabs_size;
	//uint   *map_mem;
	uint   *map;
	ushort *slabs;
	//uint	nop;

	// map:
	// id 0 : rle siz
	// id 0 : tex siz
	// id 1 : rle len
	// id 2 : rle elems ... []
	// id x : tex elems ... []
};

class Tree;

struct RLE4
{	
	/*------------------------------------------------------*/
	Map4 map[16],mapgpu[16];int nummaps;
	/*------------------------------------------------------*/
	void compress_all(Tree& tree);
	/*------------------------------------------------------*/
	Map4 compress(Tree& tree,int mip_lvl);//int sx,int sy,int sz,uchar* col1,uchar* col2);
	/*------------------------------------------------------*/
	void init();
	/*------------------------------------------------------*/
	void clear();
	/*------------------------------------------------------*/
	void save(char *filename);
	/*------------------------------------------------------*/
	bool load(char *filename);
	/*------------------------------------------------------*/
	Map4 copy_to_gpu(Map4 map4);
	/*------------------------------------------------------*/
	void all_to_gpu();
	void all_to_gpu_tex();
	/*------------------------------------------------------*/
	void setgeom (long x, long y, long z, long issolid);
	void setcol (long x, long y, long z, long argb);
	long loadvxl (char *filnam);
	Map4 compressvxl(ushort* mem,int sx,int sy,int sz,int mip_lvl);
	/*------------------------------------------------------*/
};
