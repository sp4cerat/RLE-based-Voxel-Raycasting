#pragma once

#include "core.h"
#include "vecmath.h"
#include "rle_struct.h"
#include <vector>

struct RLE
{
	struct Stick
	{
		short start;
		short end;
	};
	struct StickCol
	{
		unsigned char mat;
		unsigned char nx1,ny1,nz1;
		unsigned char mat2;
		unsigned char nx2,ny2,nz2;
	};

	struct Elem //X-Z-Element
	{
		char len;
		int stick; //offset
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

	struct Tile
	{
		int tilesize;	

		void init (int tilesize, int numsticks)
		{
			this->tilesize = tilesize;
			//this->numsticks = numsticks;

			//sticks = (Stick*)malloc ( numsticks*sizeof(Stick) );
			//map = (Elem*) malloc ( tilesize*tilesize*sizeof(Elem) );
		}
	};

	int tilesize;
	int tx,tz;
	int sx,sy,sz;

	Tile *tiles;

	/*------------------------------------------------------*/

	void export_rle2( RLE2 *rle2)
	{
		int sizemipmaps = nummipmaps*sizeof(RLE2::MipMap);
		rle2->sx = sx;
		rle2->sy = sy;
		rle2->sz = sz;
		rle2->nummipmaps = nummipmaps;
		rle2->mipmap = (RLE2::MipMap*) malloc(sizemipmaps);
		rle2->mipmap_coarse = (RLE2::MipMap*) malloc(sizemipmaps);

		memcpy(rle2->mipmap, mipmap,sizemipmaps);
		memcpy(rle2->mipmap_coarse, mipmap_coarse,sizemipmaps);

		int a,b;

		loop(a,0,nummipmaps)
		{
			int mapsize=mipmap[a].sx*mipmap[a].sz*sizeof(RLE2::Elem);
			rle2->mipmap[a].map = (RLE2::Elem*)malloc(mapsize);
			
			loop(b,0,mipmap[a].sx*mipmap[a].sz)
			{
				unsigned int elem;
				elem =  (unsigned int)mipmap[a].map[b].len;
				elem|= ((unsigned int)mipmap[a].map[b].stick)<<8;
				rle2->mipmap[a].map[b].start24_len8 = elem;	
			}

			if(mipmap_coarse[a].sticks >0)
			{
				mapsize=mipmap_coarse[a].sx*mipmap_coarse[a].sz*sizeof(RLE2::Elem);
				rle2->mipmap_coarse[a].map = (RLE2::Elem*)malloc(mapsize);
				
				loop(b,0,mipmap_coarse[a].sx*mipmap_coarse[a].sz)
				{
					unsigned int elem;
					elem =  (unsigned int)mipmap_coarse[a].map[b].len;
					elem|= ((unsigned int)mipmap_coarse[a].map[b].stick)<<8;
					rle2->mipmap_coarse[a].map[b].start24_len8 = elem;	
				}
			}
		}
	}

	/*------------------------------------------------------*/

	void gen_mipmaps()
	{
		printf("Generating Mipmaps\n");

		nummipmaps = 0;
		int test = 1;

		while ( test<sx && test<sz ) { test = test * 2; nummipmaps++; }

		mipmap = (MipMap*) malloc ( nummipmaps * sizeof(MipMap) );
		mipmap_coarse = (MipMap*) malloc ( nummipmaps * sizeof(MipMap) );

		char *tmp = (char*)malloc (5*5*sy);
		
		int a,b,c,x,y,z,d;

		mipmap_coarse_scale = 32;
		mipmap_coarse_shift = 5;

		mipmap[0].sx = sx;
		mipmap[0].sy = sy;
		mipmap[0].sz = sz;
		mipmap[0].map = map;
		mipmap[0].sticks = sticks;
		mipmap[0].numsticks = numsticks;
		mipmap[0].sticks_col = (StickCol*) malloc (mipmap[0].numsticks * sizeof(StickCol));
		printf("Level 0: %d elements (%dx%dx%d)\n",mipmap[0].numsticks,mipmap[0].sx,mipmap[0].sy,mipmap[0].sz);

		loop( x, 0, mipmap[0].numsticks )
			mipmap[0].sticks_col[x].nx1 = x & 255;

		// mipmaps
		loop( a, 1, nummipmaps   )
		{
			mipmap[a].sx = mipmap[a-1].sx / 2;
			mipmap[a].sy = mipmap[a-1].sy / 2;
			mipmap[a].sz = mipmap[a-1].sz / 2;
			mipmap[a].numsticks = 0;
			mipmap[a].map = (Elem*) malloc (mipmap[a].sx * mipmap[a].sz * sizeof(Elem));
			mipmap[a].sticks = (Stick*) malloc (mipmap[a-1].numsticks * sizeof(Stick));

			loop( x, 0, mipmap[a].sx )
			loop( z, 0, mipmap[a].sz )
			{
				Elem &elem = mipmap[a].map[x+z*mipmap[a].sx];
				elem.stick = mipmap[a].numsticks;

				memset (tmp,0,2*2*sy);
				loop( b, 0, 2 )
				loop( c, 0, 2 )
				{
					Elem& elemp = mipmap[a-1].map[(x*2+b)+(z*2+c)*mipmap[a-1].sx];

					Stick stick1,stick2; stick1.start=1;

					loop( d, elemp.stick , elemp.stick + elemp.len )
					{
						stick2=stick1; stick1 = mipmap[a-1].sticks[d];

						if( stick1.start < 0 )
						loop( y, stick2.end>>(a-1), stick1.end>>(a-1) )
						{
							tmp [y*4+b*2+c] = 1;
						}else
						loop( y, stick1.start>>(a-1), stick1.end>>(a-1) )
						{
							tmp [y*4+b*2+c] = 1;
						}
					}
				}
				int num1=0,num2=0;
				loop( y, 0, mipmap[a].sy+1 )
				{
					num2 =	num1;
					if(y!=mipmap[a].sy)
					{
						num1 =	tmp [y*8  ] + tmp [y*8+1] + tmp [y*8+2] + tmp [y*8+3] +
								tmp [y*8+4] + tmp [y*8+5] + tmp [y*8+6] + tmp [y*8+7] ;
					}
					else num1 = 0;

					int i = mipmap[a].numsticks;					

					if ( num1 >= 3 )
					if ( num2 <  3 )
					{
						mipmap[a].sticks[i].start = y << a;
					}

					if ( num1 <  3 )
					if ( num2 >= 3 )
					{
						mipmap[a].sticks[i].end = y << a;
						mipmap[a].numsticks++;
					}
				}
				elem.len = mipmap[a].numsticks - elem.stick ;
			}

			recompress_mipmap(mipmap[a]);

			mipmap[a].sticks = (Stick*) realloc ( mipmap[a].sticks , mipmap[a].numsticks * sizeof(Stick));
			mipmap[a].sticks_col = (StickCol*) malloc (mipmap[a].numsticks * sizeof(StickCol));

			loop( x, 0, mipmap[a].numsticks )
				mipmap[a].sticks_col[x].nx1 = x & 255;

			printf("Level %d: %d elements (%dx%dx%d)\n",a,mipmap[a].numsticks,mipmap[a].sx,mipmap[a].sy,mipmap[a].sz);
		}

		// gradients


		int e,f;

		int sample_size =   5;
		int sample_size2=  25;
		int sample_from =  -2;
		int sample_to   = 2+1;

//		if(0)
		loop( a, 0, nummipmaps   )
		{
			printf("Gradients for Level %d\n",a);
			loop( x, 0, mipmap[a].sx )
			loop( z, 0, mipmap[a].sz )
			{
				memset ( &tmp[ 0 ] , 0, mipmap[a].sy*sample_size2 );
				loops( b, sample_from, sample_to, 1 ) if( x+b >= 0 ) if( x+b < mipmap[a].sx )
				loops( c, sample_from, sample_to, 1 ) if( z+c >= 0 ) if( z+c < mipmap[a].sz )
				{
					Stick stick1,stick2; stick1.start=1;
					int ofs = ((b-sample_from)+(c-sample_from)*sample_size) * mipmap[a].sy;
					Elem& elem = mipmap[a].map[(x+b)+(z+c)*mipmap[a].sx];
					loop( d, elem.stick , elem.stick + elem.len )
					{
						stick2=stick1; stick1 = mipmap[a].sticks[d];

						if( stick1.start < 0 )
							memset ( &tmp[ (stick2.end>>a) + ofs ] , 1, (stick1.end-stick2.end)>>a);
						else
							memset ( &tmp[ (stick1.start>>a) + ofs ] , 1, (stick1.end-stick1.start)>>a);
					}
				}

				Elem& elem = mipmap[a].map[x+z*mipmap[a].sx];
				loop( f, elem.stick , elem.stick + elem.len )
				loop( e,  0, 2 )
				{
					bool s1l0 = (f<elem.stick + elem.len-1)&&(mipmap[a].sticks[f+1].start<0);
					bool s2l0 = mipmap[a].sticks[f].start<0;

					
					if (s1l0 && s2l0)
						y = (e==0) ? (abs(int(mipmap[a].sticks[f].start))>>a) : ((mipmap[a].sticks[f].start>>a)-1);
					else if (s2l0)
						y = (e==0) ? (abs(int(mipmap[a].sticks[f].end))>>a) : ((mipmap[a].sticks[f].end>>a)-1);
					else if (s1l0)
						y = (e==0) ? (abs(int(mipmap[a].sticks[f].start))>>a) : ((mipmap[a].sticks[f].start>>a)-1);
					else
						y = (e==0) ? (abs(int(mipmap[a].sticks[f].start))>>a) : ((mipmap[a].sticks[f].end>>a)-1);
						
			/*		if (s1l0 && s2l0)
						y = abs(int(mipmap[a].sticks[f].start))>>a;
					else if (s2l0)
						y = abs(int(mipmap[a].sticks[f].end))>>a;
					else if (s1l0)
						y = abs(int(mipmap[a].sticks[f].start))>>a;
					else
						y = ((abs(int(mipmap[a].sticks[f].start))>>a) + ((mipmap[a].sticks[f].end>>a)-1)) /2;
*/
					int centroid_x = 0;
					int centroid_y = 0;
					int centroid_z = 0;

					loops( d, sample_from, sample_to, 1 ) if( y+d >= 0 ) if( y+d < mipmap[a].sy )
					loops( b, sample_from, sample_to, 1 ) if( x+b >= 0 ) if( x+b < mipmap[a].sx )
					loops( c, sample_from, sample_to, 1 ) if( z+c >= 0 ) if( z+c < mipmap[a].sz )
					if ( tmp[((b-sample_from)+(c-sample_from)*sample_size) * mipmap[a].sy +y+d ] )
					{
						centroid_x+=b;
						centroid_y+=d;
						centroid_z+=c;
					}

					vec3f centroid( -centroid_x,centroid_y,-centroid_z );
					centroid.normalize();
					int nx = int(float(127.0f*centroid.x + 128.0f)); if (nx>255)nx=255; if (nx<0)nx=0;
					int ny = int(float(127.0f*centroid.y + 128.0f)); if (ny>255)ny=255; if (ny<0)ny=0;
					int nz = int(float(127.0f*centroid.z + 128.0f)); if (nz>255)nz=255; if (nz<0)nz=0;

						//mipmap[a].sticks_col[f].nx1 = nx;
						//mipmap[a].sticks_col[f].ny1 = ny;
						//mipmap[a].sticks_col[f].nz1 = nz;
						
					if(e==0)
					{
						mipmap[a].sticks_col[f].nx2 = nx;
						mipmap[a].sticks_col[f].ny2 = ny;
						mipmap[a].sticks_col[f].nz2 = nz;
					}

					if(e==1)
					{
						mipmap[a].sticks_col[f].nx1 = nx;
						mipmap[a].sticks_col[f].ny1 = ny;
						mipmap[a].sticks_col[f].nz1 = nz;
					}
				}
			}
		}


		// coarse preview mipmaps

		loop( a, 0, nummipmaps   ) mipmap_coarse[a].sticks = 0;

		loop( a, 0, nummipmaps   )
		if ( mipmap[a].sx >= mipmap_coarse_scale )
		if ( mipmap[a].sz >= mipmap_coarse_scale )
		{
			mipmap_coarse[a].sx = mipmap[a].sx / mipmap_coarse_scale;
			mipmap_coarse[a].sy = mipmap[a].sy ;
			mipmap_coarse[a].sz = mipmap[a].sz / mipmap_coarse_scale;
			mipmap_coarse[a].numsticks = 0;
			mipmap_coarse[a].map = (Elem*) malloc (mipmap_coarse[a].sx * mipmap_coarse[a].sz * sizeof(Elem));
			mipmap_coarse[a].sticks = (Stick*) malloc (mipmap[a].numsticks * sizeof(Stick));

			loop( x, 0, mipmap_coarse[a].sx )
			loop( z, 0, mipmap_coarse[a].sz )
			{
				int s = mipmap_coarse_scale;

				Elem &elem = mipmap_coarse[a].map[x+z*mipmap_coarse[a].sx];
				elem.stick = mipmap_coarse[a].numsticks;

				memset ( &tmp[ 0 ] , 0, mipmap[a].sy );

				loop( b, 0, s )
				loop( c, 0, s )
				{
					Elem& elemp = mipmap[a].map[(x*s+b)+(z*s+c)*mipmap[a].sx];
					Stick stick1,stick2;stick1.start=1;

					loop( d, elemp.stick , elemp.stick + elemp.len )
					{
						stick2=stick1; stick1 = mipmap[a].sticks[d];
						if( stick1.start < 0 )
							memset ( &tmp[ stick2.end>>a ] , 1, (stick1.end-stick2.end)>>a );
						else
							memset ( &tmp[ stick1.start>>a ] , 1, (stick1.end-stick1.start)>>a );
					}
				}
				int num1=0,num2=0;
				loop( y, 0, mipmap_coarse[a].sy+1 )
				{
					num2 =	num1;

					if(y!=mipmap_coarse[a].sy) num1 = tmp[y]; else num1 = 0;

					int i = mipmap_coarse[a].numsticks;					

					if ( num1 )
					if (!num2 )
					{
						mipmap_coarse[a].sticks[i].start = y << a;
					}

					if (!num1 )
					if ( num2 )
					{
						mipmap_coarse[a].sticks[i].end = y << a;
						mipmap_coarse[a].numsticks++;
					}
				}
				elem.len = mipmap_coarse[a].numsticks - elem.stick ;
			}
			mipmap_coarse[a].sticks = (Stick*) realloc ( mipmap_coarse[a].sticks , mipmap_coarse[a].numsticks * sizeof(Stick));
		//	printf("Coarse Level %d: %d elements (%dx%dx%d)\n",a,mipmap_coarse[a].numsticks,mipmap_coarse[a].sx,mipmap_coarse[a].sy,mipmap_coarse[a].sz);
		}

		loop( a, 0, nummipmaps   )
		loop( b, 0, mipmap[a].numsticks   )
			mipmap[a].sticks[b].start = abs( mipmap[a].sticks[b].start );

		free(tmp);

		printf("Levels: %d\n",nummipmaps);
	}

	/*------------------------------------------------------*/

	void get_coarse()
	{
		printf("Get Coarse\n");

		int a,b,c,d,x,y,z;

		char *tmp = (char*)malloc (5*5*sy);

		//mipmap = (MipMap*) malloc ( nummipmaps * sizeof(MipMap) );
		mipmap_coarse = (MipMap*) malloc ( nummipmaps * sizeof(MipMap) );

		mipmap_coarse_scale = 32;
		mipmap_coarse_shift = 5;

		// coarse preview mipmaps

		loop( a, 0, nummipmaps   ) mipmap_coarse[a].sticks = 0;

		loop( a, 0, nummipmaps   )
		if ( mipmap[a].sx >= mipmap_coarse_scale )
		if ( mipmap[a].sz >= mipmap_coarse_scale )
		{
			mipmap_coarse[a].sx = mipmap[a].sx / mipmap_coarse_scale;
			mipmap_coarse[a].sy = mipmap[a].sy ;
			mipmap_coarse[a].sz = mipmap[a].sz / mipmap_coarse_scale;
			mipmap_coarse[a].numsticks = 0;
			mipmap_coarse[a].map = (Elem*) malloc (mipmap_coarse[a].sx * mipmap_coarse[a].sz * sizeof(Elem));
			mipmap_coarse[a].sticks = (Stick*) malloc (mipmap[a].numsticks * sizeof(Stick));

			loop( x, 0, mipmap_coarse[a].sx )
			loop( z, 0, mipmap_coarse[a].sz )
			{
				int s = mipmap_coarse_scale;

				Elem &elem = mipmap_coarse[a].map[x+z*mipmap_coarse[a].sx];
				elem.stick = mipmap_coarse[a].numsticks;

				memset ( &tmp[ 0 ] , 0, mipmap[a].sy );

				loop( b, 0, s )
				loop( c, 0, s )
				{
					Elem& elemp = mipmap[a].map[(x*s+b)+(z*s+c)*mipmap[a].sx];
					Stick stick1,stick2;stick1.start=1;

					loop( d, elemp.stick , elemp.stick + elemp.len )
					{
						stick2=stick1; stick1 = mipmap[a].sticks[d];
						if( stick1.start < 0 )
							memset ( &tmp[ stick2.end>>a ] , 1, (stick1.end-stick2.end)>>a );
						else
							memset ( &tmp[ stick1.start>>a ] , 1, (stick1.end-stick1.start)>>a );
					}
				}
				int num1=0,num2=0;
				loop( y, 0, mipmap_coarse[a].sy+1 )
				{
					num2 =	num1;

					if(y!=mipmap_coarse[a].sy) num1 = tmp[y]; else num1 = 0;

					int i = mipmap_coarse[a].numsticks;					

					if ( num1 )
					if (!num2 )
					{
						mipmap_coarse[a].sticks[i].start = y << a;
					}

					if (!num1 )
					if ( num2 )
					{
						mipmap_coarse[a].sticks[i].end = y << a;
						mipmap_coarse[a].numsticks++;
					}
				}
				elem.len = mipmap_coarse[a].numsticks - elem.stick ;
			}
			mipmap_coarse[a].sticks = (Stick*) realloc ( mipmap_coarse[a].sticks , mipmap_coarse[a].numsticks * sizeof(Stick));
		//	printf("Coarse Level %d: %d elements (%dx%dx%d)\n",a,mipmap_coarse[a].numsticks,mipmap_coarse[a].sx,mipmap_coarse[a].sy,mipmap_coarse[a].sz);
		}

		free(tmp);

		printf("Get Coarse finish.\n");
	}
	/*------------------------------------------------------*/

	void save(char *filename)
	{
		printf("Saving %s\n",filename);

		FILE* fn; 
		
		if ((fn = fopen (filename,"wb")) == NULL) return;

		fwrite(&sx,1,4,fn);
		fwrite(&sy,1,4,fn);
		fwrite(&sz,1,4,fn);
		fwrite(&numsticks,1,4,fn);
		fwrite(map,1,sx*sz*sizeof(Elem),fn);
		fwrite(sticks,1,numsticks*sizeof(Stick),fn);
/*
		int map_size_in = sx*sz*sizeof(Elem);
		int stx_size_in = numsticks*sizeof(Stick);

		unsigned char* cpr_map = (unsigned char*) malloc( map_size_in*2 );
		unsigned char* cpr_stx = (unsigned char*) malloc( stx_size_in*2 );

		int cpr_map_len = LZ_Compress( (unsigned char*) map, cpr_map, map_size_in);
		int cpr_stx_len = LZ_Compress( (unsigned char*) sticks, cpr_stx, stx_size_in);

		fwrite(cpr_map,1,cpr_map_len,fn);
		fwrite(cpr_stx,1,cpr_stx_len,fn);
*/
		fclose(fn);
	}

	/*------------------------------------------------------*/

	void save_all(char *filename)
	{
		printf("Saving %s\n",filename);

		FILE* fn; 
		
		if ((fn = fopen (filename,"wb")) == NULL) return;

		fwrite(&nummipmaps,1,4,fn);
		fwrite(&mipmap[0],1,nummipmaps*sizeof(MipMap),fn);

		int a;
		loop( a,0,nummipmaps )
		{
			int mapsize = mipmap[a].sx * mipmap[a].sz * sizeof(Elem);
			int sticksize = mipmap[a].numsticks * sizeof(Stick);
			int stickcolsize = mipmap[a].numsticks * sizeof(StickCol);

			fwrite(&mipmap[a].map[0],1,mapsize,fn);
			fwrite(&mipmap[a].sticks[0],1,sticksize,fn);
			fwrite(&mipmap[a].sticks_col[0],1,stickcolsize,fn);
		}

		fclose(fn);
	}

	/*------------------------------------------------------*/

	void load_all(char *filename)
	{
		printf("Loading %s\n",filename);

		FILE* fn; 
		
		if ((fn = fopen (filename,"rb")) == NULL) return;

		fread(&nummipmaps,1,4,fn);
		
		mipmap=(MipMap*)malloc(nummipmaps*sizeof(MipMap));

		fread(&mipmap[0],1,nummipmaps*sizeof(MipMap),fn);

		int a;
		loop( a,0,nummipmaps )
		{
			int mapsize = mipmap[a].sx * mipmap[a].sz * sizeof(Elem);
			int sticksize = mipmap[a].numsticks * sizeof(Stick);
			int stickcolsize = mipmap[a].numsticks * sizeof(StickCol);

			mipmap[a].map=(Elem*)malloc(mapsize);
			mipmap[a].sticks=(Stick*)malloc(sticksize);
			mipmap[a].sticks_col=(StickCol*)malloc(stickcolsize);

			fread(&mipmap[a].map[0],1,mapsize,fn);
			fread(&mipmap[a].sticks[0],1,sticksize,fn);
			fread(&mipmap[a].sticks_col[0],1,stickcolsize,fn);
		}
		sx = mipmap[0].sx;
		sy = mipmap[0].sy;
		sz = mipmap[0].sz;
		numsticks = mipmap[0].numsticks;

		fclose(fn);

		get_coarse();
	}

	/*------------------------------------------------------*/

	void load_raw(char *filename)
	{
		FILE* fn;
	
		if ((fn = fopen (filename,"rb")) == NULL) return;

		int voxel_size_x=0;
		int voxel_size_y=0;
		int voxel_size_z=0;

		fread(&voxel_size_x,1,4,fn);
		fread(&voxel_size_y,1,4,fn);
		fread(&voxel_size_z,1,4,fn);

		char* data = (char*)malloc((voxel_size_x/8)*voxel_size_y*voxel_size_z);

		fread(data,1, (voxel_size_x/8)*voxel_size_y*voxel_size_z,fn);

		compress(voxel_size_x,voxel_size_y,voxel_size_z,data);

		fclose(fn);
	}
	/*------------------------------------------------------*/

	void load(char *filename)
	{
		FILE* fn; 

		if ((fn = fopen (filename,"rb")) == NULL) return;

		printf("Loading %s\n",filename);

		fread(&sx,1,4,fn);
		fread(&sy,1,4,fn);
		fread(&sz,1,4,fn);
		fread(&numsticks,1,4,fn);

		map = (Elem*) malloc ( sx*sz*sizeof(Elem) );
		sticks = (Stick*)malloc ( numsticks*sizeof(Stick) );

		fread(map,1,sx*sz*sizeof(Elem),fn);
		fread(sticks,1,numsticks*sizeof(Stick),fn);

		fclose(fn);

	//	gen_mipmaps();
	}

	/*------------------------------------------------------*/

	void uncompress(char *data)
	{
		printf("uncompressing...");

		int k,x,y,z;

		int num = 0;

		int sx8 = sx/8;

		memset (data,0,sx*sy*sz/8);

		//get max complexity
		for ( x = 0; x < sx ; x++ )
		for ( z = 0; z < sz ; z++ )
		{
			Elem &elem = map[ x+z*sx ];
			
			int pre1 = (x)/8+(z)*sx8*sy;
			unsigned char pre2 = 1<<(x&7);

			for ( k = elem.stick ; k < elem.stick + elem.len ; k++ )
			{
				if( sticks[k].start < 0 )
					loop (y,sticks[k-1].end,sticks[k].end) data[pre1+y*sx8] |= pre2;
				else
					loop (y,sticks[k].start,sticks[k].end) data[pre1+y*sx8] |= pre2;
			}
		}
	}

	/*------------------------------------------------------*/

	void recompress_mipmap(MipMap &m)
	{

		if (m.sx<16) return;
		if (m.sz<16) return;

		printf("uncompressing mipmap...\n");

		int k,x,y,z;

		int num = 0;

		int sx8 = m.sx/8;

		int scale=sy/m.sy;

		//char* data=(char*)malloc(sx8*m.sy*m.sz);
		std::vector<char> data;		
		data.resize(sx8*m.sy*m.sz);
		
		memset (&data[0],0,sx8*m.sy*m.sz);

		//get max complexity
		for ( x = 0; x < m.sx ; x++ )
		for ( z = 0; z < m.sz ; z++ )
		{
			Elem &elem = m.map[ x+z*m.sx ];
			
			int pre1 = (x)/8+(z)*sx8*m.sy;
			unsigned char pre2 = 1<<(x&7);

			for ( k = elem.stick ; k < elem.stick + elem.len ; k++ )
			{
				if( k > 0 && m.sticks[k].start < 0 )
					loop (y,m.sticks[k-1].end/scale,m.sticks[k].end/scale) data[pre1+y*sx8] |= pre2;
				else
					loop (y,m.sticks[k].start/scale,m.sticks[k].end/scale) data[pre1+y*sx8] |= pre2;
			}
		}

		free (m.sticks);
		
		printf("compressing mipmap...\n");

		m.numsticks = 0;

		Stick tmp_stick;
		std::vector<Stick> tmp_sticks;
		tmp_sticks.clear();
		
		//get max sticks
		for ( x = 0; x < m.sx ; x++ )
		for ( z = 0; z < m.sz ; z++ )
		{
			Elem &elem = m.map[ x+z*m.sx ];
			elem.stick = -1;
			elem.len   = 0;

			int pre1 = (x)/8+(z)*sx8*m.sy;
			unsigned char pre2 = 1<<(x&7);

			unsigned char a=0;
			unsigned char b=0;

			bool inside = false;

			int numset1=0,numset2=0;

			for ( y = 0; y < m.sy+1 ; y++ )
			{
				b = a;

				if(y==m.sy) a=0; 
				else	  a = data[pre1+y*sx8] & pre2;

				if(!inside)if( (b==0) || (y==0) ) if ( a!=0 )
				{
					tmp_stick.start=y*scale;
					tmp_stick.end=y*scale;
					inside = true;
				}
				if(inside)if(y>0)if(b!=0)if(a==0)
				{
					elem.len++;
					if(elem.stick==-1) elem.stick = (int)tmp_sticks.size();

					tmp_stick.end = y*scale;
					tmp_sticks.push_back(tmp_stick);
					inside = false;
				}

				numset2 = numset1;
				numset1 = 0;

				if (a) if(b) 
				if (y>=0) if(y<m.sy) 
				if (x>=0) if(x<m.sx) 
				if (z>=0) if(z<m.sz) 
				{
					int c,d,e;
					if (y<m.sy-1)if(data[pre1+(y+1)*sx8] & pre2)
					if(x>0)if(y>0)if(z>0)
					if (z<m.sz-1)if (x<m.sx-1)
					loop ( c , -1 , 2 ) 
					loop ( d , -1 , 2 ) 
					loop ( e , -1 , 2 ) 
					{
						int ofs = (x+c)/8+(y+d)*sx8+(z+e)*sx8*m.sy;
						int and = 1<<((x+c)&7);
						if( (data[ofs] & and) != 0) numset1++;
					}
					if ( inside )if ( numset1 == 3*3*3 ) if ( numset2 < 3*3*3 )
					{
						elem.len++;
						if(elem.stick==-1) elem.stick = (int)tmp_sticks.size();
						tmp_stick.end = y*scale;
						tmp_sticks.push_back(tmp_stick);
						inside = false;
					}
					if ( !inside )if ( numset1 < 3*3*3 ) if ( numset2 == 3*3*3 )
					{
						tmp_stick.start=-y*scale;
						tmp_stick.end=y*scale;
						inside = true;
					}

				}
			}
		}

		m.numsticks = (int)tmp_sticks.size();
		m.sticks = (Stick*) malloc ( numsticks*sizeof(Stick) );
		
		int a;
		loop( a , 0 , m.numsticks )
			m.sticks[a] = tmp_sticks[a];

		data.clear();
	}

	/*------------------------------------------------------*/

	void compress(int sx,int sy,int sz,char *data)
	{
		printf("Compressing RLE....\n");

		this->sx = sx;
		this->sy = sy;
		this->sz = sz;

		int sx8 = sx/8;

		int x,y,z;

		map = (Elem*) malloc ( sx*sz*sizeof(Elem) );

		numsticks = 0;

		Stick tmp_stick;
		std::vector<Stick> tmp_sticks;
		tmp_sticks.clear();
		
		//get max sticks
		for ( x = 0; x < sx ; x++ )
		for ( z = 0; z < sz ; z++ )
		{
			Elem &elem = map[ x+z*sx ];
			elem.stick = -1;
			elem.len   = 0;

			int pre1 = (x)/8+(z)*sx8*sy;
			unsigned char pre2 = 1<<(x&7);

			static unsigned char a=0;
			static unsigned char b=0;

			a=b=0;

			bool inside = false;

			int numset1=0,numset2=0;

			for ( y = 0; y < sy+1 ; y++ )
			{
				b = a;

				if(y==sy) a=0; 
				else	  a = data[pre1+y*sx8] & pre2;

				if(!inside)if( (b==0) || (y==0) ) if ( a!=0 )
				{
					tmp_stick.start=y;
					tmp_stick.end=y;
					inside = true;
				}
				if(inside)if(y>0)if(b!=0)if(a==0)
				{
					elem.len++;
					if(elem.stick==-1) elem.stick = (int)tmp_sticks.size();

					tmp_stick.end = y;
					tmp_sticks.push_back(tmp_stick);
					inside = false;
				}

				numset2 = numset1;
				numset1 = 0;

				if (a) if(b) 
				if (y>=0) if(y<sy) 
				if (x>=0) if(x<sx) 
				if (z>=0) if(z<sz) 
				{
					int c,d,e;
					if (y<sy-1)if(data[pre1+(y+1)*sx8] & pre2)
					if(x>0)if(y>0)if(z>0)
					if (z<sz-1)if (x<sx-1)
					loop ( c , -1 , 2 ) 
					loop ( d , -1 , 2 ) 
					loop ( e , -1 , 2 ) 
					{
						int ofs = (x+c)/8+(y+d)*sx8+(z+e)*sx8*sy;
						int and = 1<<((x+c)&7);
						if( (data[ofs] & and) != 0) numset1++;
					}
					if ( inside )if ( numset1 == 3*3*3 ) if ( numset2 < 3*3*3 )
					{
						elem.len++;
						if(elem.stick==-1) elem.stick = (int)tmp_sticks.size();
						tmp_stick.end = y;
						tmp_sticks.push_back(tmp_stick);
						//index ++;
						inside = false;
					}
					if ( !inside )if ( numset1 < 3*3*3 ) if ( numset2 == 3*3*3 )
					{
						tmp_stick.start=-y;
						tmp_stick.end=y;
						inside = true;
					}

				}
			}
		}

		numsticks = (int)tmp_sticks.size();
		sticks = (Stick*) malloc ( numsticks*sizeof(Stick) );
		
		int a;
		loop( a , 0 , numsticks )
			sticks[a] = tmp_sticks[a];
	}

	/*------------------------------------------------------*/
};


