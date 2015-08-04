// --------------------------------------- //
// includes, system
#include "rle3.h"
#include "bmp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "../test/bmalloc.h"

// --------------------------------------- //

void RLE3::Block::init(int sx,int sy,int sz)
{
	sizex = sx;
	sizey = sy;
	sizez = sz;
	map = (uint*) malloc(sizex*sizey*4);
	if(map==0){printf("nomem\n");while(1);}
}

// --------------------------------------- //

void RLE3::Block::save(char* filename)
{
	printf("Saving Block %s\n",filename);

	FILE* fn; 
	
	if ((fn = fopen (filename,"wb")) == NULL) return;

	fwrite(&slabs_size,1,4,fn);
	fwrite(&sizex,1,4,fn);
	fwrite(&sizey,1,4,fn);
	fwrite(&sizez,1,4,fn);

	if(slabs_size>0)
	{
		fwrite(&map[0],1,sizex*sizey*4,fn);
		fwrite(&slabs[0],1,slabs_size*2,fn);
	}
	fclose(fn);
}

// --------------------------------------- //

void RLE3::Block::cleanup()
{
	if(map)free(map);
	if(slabs)free(slabs);
}

// --------------------------------------- //

void RLE3::Block::load(char* filename)
{
	printf("Reading Block %s\n",filename);

	FILE* fn; 
	
	if ((fn = fopen (filename,"rb")) == NULL) {printf("file not found");while(1);;}

	fread(&slabs_size,1,4,fn);
	fread(&sizex,1,4,fn);
	fread(&sizey,1,4,fn);
	fread(&sizez,1,4,fn);

	if(slabs_size>0)
	{
		map = (uint*) malloc ( sizex*sizey*4) ;
		fread(&map[0],1,sizex*sizey*4,fn);
		slabs = (ushort*) malloc (slabs_size*2) ;
		fread(&slabs[0],1,slabs_size*2,fn);

		if(map==0){printf("nomem\n");while(1);}
		if(slabs==0){printf("nomem\n");while(1);}
	}
	fclose(fn);
}

// --------------------------------------- //

void* RLE3::Block::load_gpu(char* filename)
{
	printf("Reading Block to Pool %s\n",filename);

	FILE* fn; 
	
	if ((fn = fopen (filename,"rb")) == NULL) 
	{
		printf("%s not found error !\n",filename);
		while(1);;
	};

	int sx,sy,sz,slabssize;

	fread(&slabssize,1,4,fn);
	fread(&sx,1,4,fn);
	fread(&sy,1,4,fn);
	fread(&sz,1,4,fn);

	int mem_size=16+( sx*sy*4 )+(slabssize*2);
	char* mem=(char*) bmalloc(mem_size);

	if (mem==0) { printf("\nno mem\n");while(1);}

	((int*)mem)[0]=slabssize;
	((int*)mem)[1]=sx;
	((int*)mem)[2]=sy;
	((int*)mem)[3]=sz;

	sizex = sx;
	sizey = sy;
	sizez = sz;
	
//	uint   *map;
//	ushort *slabs;

	if(slabssize>0)
	{
		map = (uint*) (mem+16);//(uint*) bmalloc (sizex*sizey*4) ;
		fread(&map[0],1,sx*sy*4,fn);

		slabs = (ushort*) (mem+16+( sx*sy*4 ));;//(ushort*) bmalloc (slabs_size*2) ;
		fread(&slabs[0],1,slabssize*2,fn);
	}
	fclose(fn);

//	uint   *map; // s 24 l 8
//	ushort *slabs;int slabs_size;
//	ushort *scols;int scols_size;

//	static char* text[2000];
//	sprintf((char*)text,"%s.bmp",filename);
//	save_bmp((char*)text);

	gpu_memcpy(mem+cpu_to_gpu_delta,mem,mem_size);

	return (mem+cpu_to_gpu_delta);
}

// --------------------------------------- //

void RLE3::Block::save_bmp(char* name)
{
	int sx = sizex;
	int sy = sizey;
	int sz = sizez;

	//check

	Bmp bmp (sx,sz,24,0);

	memset(bmp.data,0,sx*sz*3);

	int x,z ;

	loop(x,0,sx)
	{
		int z1,z2;

		uint zstart16 = map[x+1*sx]/256;
		uint zlen16   = map[x+1*sx]&255;

		uint ofs=zlen16+zstart16;

		loop(z1,0,zlen16) // 256
		{
			uint a=slabs[zstart16+z1];

			uint start8=a&0xff00;
			uint len8  =a&255;

			loop(z2,0,len8) // 8
			{
				uint a = slabs[ofs++];
				uint start=start8+(a>>8);
				uint len  = a&255;

				if(start+len>sz) 
				{
					continue;
				}

				loop(z,start,start+len) // 1
				{
					uint o=(z*sx+x)*3;
					bmp.data[o+0]=
					bmp.data[o+1]=
					bmp.data[o+2]=255;
				}
			}
		}
	}
	bmp.save(name);
}

// --------------------------------------- //

void RLE3::Block::uncompress(uchar* mem)
{
	int sx = sizex;
	int sy = sizey;
	int sz = sizez;

	//check

	memset(mem,0,sx*sy*sz);

	int x,y,z ;

	loop(y,0,sy)
	loop(x,0,sx)
	{
		int z1,z2;

		uint zstart16 = map[x+y*sx]/256;
		uint zlen16   = map[x+y*sx]&255;

		uint ofs=zlen16+zstart16;

		loop(z1,0,zlen16) // 256
		{
			uint a=slabs[zstart16+z1];

			uint start8=a&0xff00;
			uint len8  =a&255;

			loop(z2,0,len8) // 8
			{
				uint a = slabs[ofs++];
				uint start=start8+(a>>8);
				uint len  = a&255;

				if(start+len>sz) 
				{
					continue;
				}

				loop(z,start,start+len) // 1
				{
					mem[y*sx+x+z*sx*sy]=255;
				}
			}
		}
	}
}

// --------------------------------------- //

void RLE3::Block::compress(uchar* mem,int sx,int sy,int sz)
{

	this->init(sx,sy,sz);

	int x,y,z;

	printf("Compressing\n");

	std::vector<ushort> slabs_complete;
	std::vector<ushort> start16;
	std::vector<ushort> slabs;
	//std::vector<SCol> scols;

	loop(x,0,sx)//256
	{
		printf("Compressing %d \%\r",x*100/(sx));
		loop(y,0,sy)//256
		{
			bool inside = false;
			int slab_start;
			int slab_len;
//			SCol scol;

			start16.clear();
			slabs.clear();
//			scols.clear();
			
			loop(z,0,sz+1)			
			{
				bool iso = false;
				
				if (z<sz) iso = (mem[z*sx*sy+y*sx+x]>60) ? true : false;

				if(inside)
				{
					if ( z-slab_start == 255  )
					{
						inside=false;
						slab_len=z-slab_start;
						slabs.push_back( ((slab_start & 255) <<8 ) + (slab_len&255) );
						start16[start16.size()-1]++;
					}
					//continue;
				}
				if(iso && !inside) // out>>in
				{
					inside=true;
					slab_start=z;
			
					if (start16.size()==0) 
					{
						start16.push_back((slab_start & 0xff00));
					}
					else
					if(((start16[start16.size()-1]>>8)!=(z>>8)))
					{
						start16.push_back((slab_start & 0xff00));
					}
					continue;
				}
				if(!iso && inside) // in>>out
				{
					inside=false;
					slab_len=z-slab_start;
					slabs.push_back( ((slab_start & 255)<<8) + (slab_len & 255));
					start16[start16.size()-1]++;
					continue;
				}
			}

			uint size1 = (uint)slabs_complete.size();
			uint size2 = (uint)start16.size();
			uint size3 = (uint)slabs.size();
			slabs_complete.resize( size1+size2+size3 );

			if (size2>0) memcpy( &slabs_complete[size1], &start16[0], 2*size2);
			if (size3>0) memcpy( &slabs_complete[size1+size2], &slabs[0], 2*size3);

			this->map[x+y*sx] = size1*256+size2;
		}
	}
	if(slabs_complete.size()==0) {printf("\nempty !!?\n");return;}

	this->slabs = (ushort*) malloc(slabs_complete.size()*2) ;
	memcpy(this->slabs,(ushort*) &slabs_complete[0],slabs_complete.size()*2) ;
	this->slabs_size = (uint)slabs_complete.size();
}


// --------------------------------------- //

void RLE3::load_richtmyer_block(uchar* mem , int brick_num)
{
	char filename[1000];
	FILE* fn; 
	sprintf(filename,"C:/voldata/d_0239_%04d",brick_num);
//		sprintf(filename,"D:/VolumeData/d_0219_%04d",num);
	
	if ((fn = fopen (filename,"rb")) == NULL) return;
	printf("loading %s\n",filename);
	const int blocksize =1024*1024*8;
	fread(mem,1,blocksize,fn);
	fclose(fn);
}

// --------------------------------------- //

void RLE3::compress_block(uchar* mem,int sx,int sy,int sz,int id)
{

	Block block;
	block.init(sx,sy,sz);

	int x,y,z;

	printf("Compressing\n");

	std::vector<ushort> slabs_complete;
	std::vector<ushort> start16;
	std::vector<ushort> slabs;
	//std::vector<SCol> scols;

	loop(x,0,sx)//256
	{
		printf("Compressing %d \%\r",x*100/(sx-1));
		loop(y,0,sy)//256
		{
			bool inside = false;
			int slab_start;
			int slab_len;
//			SCol scol;

			start16.clear();
			slabs.clear();
//			scols.clear();
			
			loop(z,0,sz+1)			
			{
				bool iso = false;
				
				if (z<sz) iso = (mem[z*65536+y*256+x]>60) ? true : false;

				if(inside)
				{
					if ( z-slab_start == 255  )
					{
						inside=false;
						slab_len=z-slab_start;
						slabs.push_back( ((slab_start & 255) <<8 ) + (slab_len&255) );
						start16[start16.size()-1]++;
					}
					//continue;
				}
				if(iso && !inside) // out>>in
				{
					inside=true;
					slab_start=z;
			
					if (start16.size()==0) 
					{
						start16.push_back((slab_start & 0xff00));
					}
					else
					if(((start16[start16.size()-1]>>8)!=(z>>8)))
					{
						start16.push_back((slab_start & 0xff00));
					}
					continue;
				}
				if(!iso && inside) // in>>out
				{
					inside=false;
					slab_len=z-slab_start;
					slabs.push_back( ((slab_start & 255)<<8) + (slab_len & 255));
					start16[start16.size()-1]++;
					continue;
				}
			}

			uint size1 = (uint)slabs_complete.size();
			uint size2 = (uint)start16.size();
			uint size3 = (uint)slabs.size();
			slabs_complete.resize( size1+size2+size3 );

			if (size2>0) memcpy( &slabs_complete[size1], &start16[0], 2*size2);
			if (size3>0) memcpy( &slabs_complete[size1+size2], &slabs[0], 2*size3);

			block.map[x+y*sx] = size1*256+size2;
		}
	}

	//bmp.save("test128.bmp"); printf("\nsaved bmp\n");

	if(slabs_complete.size()==0) {printf("\nempty !!?\n");return;}

	block.slabs = (ushort*) &slabs_complete[0];
	block.slabs_size = (uint)slabs_complete.size();

	char txt[1000]; 
	sprintf(txt,"test-%04d.rle3",id);
	block.save(txt);

	//block.save_bmp(txt);
}

// --------------------------------------- //

void RLE3::recompress_after_load()
{
	int blockexp  = 256;
	int blocksize = 1<<blockexp;
	int size      = 1024;

	for (int x=0;x<2048;x++)
	for (int y=0;y<2048;y++)
	{
		int bx = (x>>8) & 7;
		int by = (y>>8) & 7;
		int ox = (x) & 2047;
		int oy = (y) & 2047;
	}
}


// --------------------------------------- //

void RLE3::import_richtmyer()
{
	const int blocksize =1024*1024*8;
	int xb,yb,zb;
//	int xc,yc,zc;

	static uchar* mem = (uchar*)malloc(blocksize*2*8);
	if(mem==0){printf("nomem\n");while(1);}

	loop(xb,0,8)//0-7
	loop(yb,0,8)//0-7
	{
		loop(zb,0,7)
		{
			int 
			block = zb*128+yb*8+xb;
			load_richtmyer_block(mem+blocksize*(0+2*zb),block);
			block += 64;
			load_richtmyer_block(mem+blocksize*(1+2*zb),block);
		}
		compress_block(mem,256,256,7*256,xb+yb*8);
		printf("ready - %04d.\n",xb+yb*8);
	}
	printf("ALL ready.\n");
}

// --------------------------------------- //

void RLE3::load_grid(int sx,int sy,char* prefix)
{
	gridx = sx;
	gridy = sy;
	grid  = (uint**) malloc(sx*sy*sizeof(Block));
	if(grid==0){printf("nomem\n");while(1);}

	int i;
	char s[100];
	
	loop(i,0,sx*sy)
	{
		sprintf(s,"%s%04d.rle3",prefix,i);
		((Block*)grid)[i].load(s);
	}
}

// --------------------------------------- //

void RLE3::load_grid_gpu(int sx,int sy,char* prefix)
{
	gridx = sx;
	gridy = sy;
	grid  = (uint**) bmalloc(sx*sy*sizeof(int)*8);

	int i,j;
	char s[100];
	
	loop(j,0,8)
	loop(i,0,sx*sy)
	{
		sprintf(s,"%s%04d-%02d.rle3",prefix,i,j);

		Block b;
		//((uint**)grid)[i+32+j*gridx*gridy]=
		((uint**)grid)[i+j*gridx*gridy]=(uint*)b.load_gpu(s);
	}
	gpu_memcpy(((char*)grid)+cpu_to_gpu_delta,grid,gridx*gridy*4*8);

	grid = (uint**)(((char*)grid)+cpu_to_gpu_delta);
}

// --------------------------------------- //

uchar* RLE3::Block::genmipmap(uchar* mem_in, int sx_in,int sy_in,int sz_in )
{
	int sx = sx_in/2;
	int sy = sy_in/2;
	int sz = sz_in/2;
	
	uchar* mem = (uchar*)malloc(sx*sy*sz);

	int x,y,z,w;

	loop(x,0,sx)
	loop(y,0,sy)
	loop(z,0,sz)
	{
		uchar a = 0;

		static const int addx[8]={0,0,0,0,1,1,1,1};
		static const int addy[8]={0,0,1,1,0,0,1,1};
		static const int addz[8]={0,1,0,1,0,1,0,1};

		loop(w,0,8)
			if (mem_in[	(x*2+addx[w])+
						(y*2+addy[w])*sx_in+
						(z*2+addz[w])*sx_in*sy_in] ) a++;

		mem[x+y*sx+z*sx*sy] = (a>3) ? 255 : 0;
	}

	printf("mipmap ready - compressing\n");

	compress( mem , sx,sy,sz );

	return mem;
}

// --------------------------------------- //

void RLE3::Block::reduce_to_surface()
{
	printf("malloc\n");
	uchar *mem_i = (uchar*)malloc (sizex * sizey * sizez );
	uchar *mem_o = (uchar*)malloc (sizex * sizey * sizez );

	printf("uncompress\n");
	uncompress(mem_i);

	printf("memcpy\n");
	memcpy(mem_o,mem_i,sizex * sizey * sizez);

	printf("surface\n");
	/*
	for (int x=1;x<sizex-1;x++)
	for (int y=1;y<sizey-1;y++)
	for (int z=1;z<sizez-1;z++)
	{
		int c=0;

		int o = x+(y+z*sizey)*sizex;

		for (int xx=-1;xx<=1;xx++)
		for (int yy=-1;yy<=1;yy++)
		for (int zz=-1;zz<=1;zz++)
			if (mem_i[o+xx+(zz*sizey+yy)*sizex]==0) goto con;

		mem_o[o]=0;		

		con:;
	}
	*/

	if(map)free(map);
	if(slabs)free(slabs);

	printf("compress\n");
	compress( mem_o , sizex , sizey , sizez );

	free (mem_i);
	free (mem_o);

	save_bmp("test123.bmp");
}

// --------------------------------------- //

void RLE3::Block::gen_all_mipmaps(char* prefix)
{
	char text[200];
	sprintf((char*)text,"%s.rle3",prefix);
	load((char*)text);

	uchar *mem = (uchar*)malloc (sizex * sizey * sizez );
	if(mem==0){printf("nomem\n");while(1);}

	int iteration = 0;

	sprintf((char*)text,"%s-%02d.rle3",prefix,iteration);
	printf("save rle\n");
	save((char*)text);
	iteration++;

	while( sizex > 1 && sizey > 1 && sizez > 1)
	{
		printf("-----------\n");
		printf("iteration:%d - %d %d %d\n",iteration, sizex , sizey , sizez);
		printf("uncompress\n");
		uncompress(mem);
		printf("compress\n");
		uchar *mem2 = genmipmap(mem, sizex , sizey , sizez ); free(mem); mem = mem2;
		sprintf((char*)text,"%s-%02d.rle3",prefix,iteration);
		printf("save rle\n");
		save((char*)text);
		sprintf((char*)text,"%s-%02d.bmp",prefix,iteration);
		printf("save bmp\n");
		save_bmp((char*)text);
		iteration++;
	}
}