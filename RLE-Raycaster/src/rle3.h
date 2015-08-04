// --------------------------------------- //
#pragma once
#include "Core.h"

#ifndef IN_CUDA_ENV
#include <vector>
#include "VecMath.h"
#else
typedef float3 vec3f;
typedef int3 vec3i;
#endif


//#include "vecmath.h"
/*
struct Map4
{
	int    sx,sy,sz;
	uint   *map; // 
	ushort *slabs;int slabs_size;
};

struct RLE4
{	
	Map4 map4;

	void compress(uchar* mem, int sx,int sy,int sz)
	{
		map4.sx=sx;
		map4.sy=sy;
		map4.sz=sz;
		int sxy = sx*sy;

		std::vector<ushort> slab;
		
		for (int i=0;i<sx;i++)
		for (int j=0;j<sy;j++)
		for (int k=0;k<sz;k++)
		{
			uchar f=mem[(i+j*sx+k*sxy)>>3] & (1<<(i&7));

			if(!f) // zero
			{
				if(slab.size()==0)
					slab.push_back(256);
				else
				{
					int elem=slab.back();

					if((elem&255)==0)
					{
						if((elem>>16)<255)
							slab.back()=elem+256;
						else
							slab.push_back(256);
					}
					else
						slab.push_back(256);
				}
			}
			else // not zero
			{
				int cnt=0;
				int mx=0;
				int my=0;
				int mz=0;

				for (int a=-1;a<2;a++)
				for (int b=-1;b<2;b++)
				for (int c=-1;c<2;c++)
				{
					int x=i+a;
					int y=j+b;
					int z=k+c;

					if (x<0)continue;
					if (y<0)continue;
					if (z<0)continue;
					if (x>=sx)continue;
					if (y>=sy)continue;
					if (z>=sz)continue;

					uchar d=mem[(x+y*sx+z*sxy)>>3] & (1<<(x&7));

					if(d)
					{
						mx+=a;
						my+=b;
						mz+=c;
						cnt++;
					}
				}

				if (cnt!=9)
				{
					vec3f centroid( -mx,my,-mz );
					centroid.normalize();
					int nx = int(float(127.0f*centroid.x + 128.0f)); if (nx>255)nx=255; if (nx<0)nx=0;
					int ny = int(float(127.0f*centroid.y + 128.0f)); if (ny>255)ny=255; if (ny<0)ny=0;
					int nz = int(float(127.0f*centroid.z + 128.0f)); if (nz>255)nz=255; if (nz<0)nz=0;
					
				}
			}
		}
	}
};
*/
struct RLE3
{	
//	struct SLab{uchar start,len;};
//	struct SCol{uchar color,normal;};
	
	struct Block
	{
		int    sizex,sizey,sizez;
		uint   *map; // s 24 l 8
		ushort *slabs;int slabs_size;
		ushort *scols;int scols_size;

		void init(int sx,int sy,int sz);
		void save(char* filename);
		void load(char* filename);
		void* load_gpu(char* filename);
		void save_bmp(char* filename);
		void compress(uchar* mem,int sx,int sy,int sz);
		void uncompress(uchar* mem);
		uchar* genmipmap(uchar* mem, int sx,int sy,int sz );
		void gen_all_mipmaps(char* prefix);
		void reduce_to_surface();
		void cleanup();
	};

	int gridx,gridy;
	uint** grid;

	void load_grid(int sx,int sy,char* prefix);
	void load_grid_gpu(int sx,int sy,char* prefix);
	void compress_block(uchar* mem,int sx,int sy,int sz,int id);
	void load_richtmyer_block(uchar* mem , int brick_num);
	void import_richtmyer();
	void recompress_after_load();
};

