#pragma once
#include "core.h"
#include "rle4.h"
#include <vector>
#include "VecMath.h"

class Tree
{
public :

	/*------------------------------------------------------*/
	static std::vector<float> vertex_arr;
	static std::vector<float> normal_arr;
	static std::vector<int> faces_arr;
	/*------------------------------------------------------*/
	char *voxel;
	uchar *voxel_col1;
	uchar *voxel_col2;
	int vx,vy,vz;

	char color;
	/*------------------------------------------------------*/	
	void get_mipmap(Tree& t){

		init(t.vx/2,t.vy/2,t.vz/2,(t.voxel_col1) ? 1 : 0 );

		if(faces_arr.size()>0)
		{
			for(int i=0;i<faces_arr.size()/3;i++)
			{
				if(((i/9)&8191)==0)printf("Triangle %d     \r",i);
				drawPLY(i);
			}
			return;
		}


		int sx = vx;
		int sy = vy;
		int sz = vz;
		int sxy = sx*sy;

		for (int i=0;i<vx;i++)
		for (int j=0;j<vy;j++)
		for (int k=0;k<vz;k++)
		{
			int ii=i*2;
			int jj=j*2;
			int kk=k*2;
			bool bit0=0,bit1=0,bit2=0;

			int mincnt=3;

			for(int subx=0;subx<2&&!bit0;subx++)
			for(int suby=0;suby<2&&!bit0;suby++)
			for(int subz=0;subz<2&&!bit0;subz++)
			{
				ii=i*2+subx;
				jj=j*2+suby;
				kk=k*2+subz;
				bool bitx=(bool)(t.voxel [(ii+jj*sx*2+kk*4*sxy)>>3] & (1<<(ii&7)));
				if(bitx)
				{
					mincnt--;
					if(mincnt==0)
					{
						bit0 = 1;
						if(t.voxel_col1)
						{
							bit1 = t.voxel_col1[(ii+jj*sx*2+kk*4*sxy)>>3] & (1<<(ii&7));
							bit2 = t.voxel_col2[(ii+jj*sx*2+kk*4*sxy)>>3] & (1<<(ii&7));
						}
						break;
					}
				}
			}

			if(bit0) voxel     [(i+j*sx+k*sxy)>>3]	|= (1<<(i&7)); else continue;
			if(t.voxel_col1)
			{
				if(bit1) voxel_col1[(i+j*sx+k*sxy)>>3]	|= (1<<(i&7));
				if(bit2) voxel_col2[(i+j*sx+k*sxy)>>3]	|= (1<<(i&7));
			}
		}
	}
	/*------------------------------------------------------*/
	void set_color(char c){color=c;}
	/*------------------------------------------------------*/
	bool load(char* filename,int x,int y,int z,int bpp=8,int iso=168)//36
	{
		FILE* fn; 

		if ((fn = fopen (filename,"rb")) == NULL) return false;

		printf("Loading %s\n",filename);

		uchar* mem=(uchar*)malloc(x*y*z*(bpp/8));

		int add=0;
		int shift=0;
		if(bpp==16)
		{
			add=1;shift=1;
		}

		fread(mem,1,x*y*z*(bpp/8),fn);
		fclose(fn);

		init(x,(y<x) ? x:y ,z);

		int numvox=0;

		int sx = vx;
		int sy = vy;
		int sz = vz;
		int sxy = sx*sy;
		for (int i=0;i<vx;i++)
		for (int j=0;j<vy;j++)
		for (int k=0;k<vz;k++)
		{
			int m = mem[(((i)+(k*y/vz)*x*z+(j)*x)<<shift)+add];
//			int m = mem[(((i*2/3)+(k*y/vz)*x*z+(j*3/4+vy/4)*x)<<shift)+add]; bonsai
			bool bit0 = ( m > iso) ? true : false;

			//bool bit0 = ( /*mem[(i+j*sx+k*sxy)] > iso*/ (((i^k)&j)&511) < 97) ? true : false;

			int col=1;
		//	if (m<45)if(j<vy/2) col=0;

			bool bit1 =  col&1;
			bool bit2 = (col>>1)&1;

			if(bit0){voxel     [(i+j*sx+k*sxy)>>3]	|= (1<<(i&7));numvox++;}
			if(voxel_col1)
			{
				if(bit1) voxel_col1[(i+j*sx+k*sxy)>>3]	|= (1<<(i&7));
				if(bit2) voxel_col2[(i+j*sx+k*sxy)>>3]	|= (1<<(i&7));
			}
		}

		if(numvox==0){printf("0 voxels !\n");while(1);;}

		return true;
	}
	/*------------------------------------------------------*/
	void init (int sx,int sy,int sz,bool usecolor=true)
	{
		vx = sx ; vy = sy ; vz = sz ; 
		voxel      = (char*)malloc((sx/8)*sy*sz);
		if(voxel==0)::exit(0);//MessageBoxA(0,"voxel      = (char*)malloc((sx/8)*sy*sz);","out of ram",0);
		memset(voxel,0,(sx/8)*sy*sz);

		if(usecolor)
		{
			voxel_col1 = (uchar*)malloc((sx/8)*sy*sz);
			voxel_col2 = (uchar*)malloc((sx/8)*sy*sz);
			memset(voxel_col1,0,(sx/8)*sy*sz);
			memset(voxel_col2,0,(sx/8)*sy*sz);
		}
		else
		{
			voxel_col1 =voxel_col2 = 0;
		}
	}
	/*------------------------------------------------------*/
	void exit()
	{
		free(voxel);
		if(voxel_col1)free(voxel_col1);
		if(voxel_col2)free(voxel_col2);
	}
	/*------------------------------------------------------*/
	float getRnd ()
	{
		static int seed = 4534543;
		seed = seed*754743523 - seed*7546 + seed*346 - 93337524 + seed/2345645 - seed/2353;
		return ((float)(abs(seed&511))/255)-1;
	}
	/*------------------------------------------------------*/
	void sphere(vec3f pos,float r,int mode = 0)
	{
		/*
		if ( pos.x+r > vx ) return;
		if ( pos.y+r > vy ) return;
		if ( pos.z+r > vz ) return;
		if ( pos.x-r < 0  ) return;
		if ( pos.y-r < 0  ) return;
		if ( pos.z-r < 0  ) return;
		*/

		int px=int(pos.x);
		int py=int(pos.y);
		int pz=int(pos.z);
		int r2 = int(r*r);
		int vx8 = vx/8;
		int vx8vy = vx8*vy;

		int x,y,z;

		loop ( x , int(pos.x-r), int(pos.x+r) )
		{
			int xx=x&(vx-1);int x2 = (px-x)*(px-x);
			loop ( y , int(pos.y-r), int(pos.y+r) )
			{
				int yy=y&(vy-1);int y2 = (py-y)*(py-y);
				if(y>0)
				if(y<vy)
				loop ( z , int(pos.z-r), int(pos.z+r) )
				{
					int zz=z&(vz-1);
					int a = x2+y2+(pz-z)*(pz-z);
					if (a <= r2)
					{
						int bit = 1<<(x&7);
						int ofs = (xx>>3) + yy*vx8 + zz * vx8vy;

						if (mode==0){ 
							voxel[ofs]|=bit;		// mode 0 : add

							if(voxel_col1)
							{
								voxel_col1[ofs]&=255-bit;
								voxel_col2[ofs]&=255-bit;
								bit = x&7;
								voxel_col1[ofs]|=(color&1)<<bit;		// mode 0 : add
								voxel_col2[ofs]|=(color>>1)<<bit;		// mode 0 : add
							}
						}
						if (mode==1) voxel[ofs]&=255-bit;	// mode 1 : sub
					}
				}
			}
		}
	}
	/*------------------------------------------------------*/
	void cube(vec3f pos1,vec3f pos2)
	{
		if ( pos2.x > vx ) return;
		if ( pos2.y > vy ) return;
		if ( pos2.z > vz ) return;
		if ( pos1.x < 0  ) return;
		if ( pos1.y < 0  ) return;
		if ( pos1.z < 0  ) return;

		int px=int(pos1.x);
		int py=int(pos1.y);
		int pz=int(pos1.z);
		int vx8 = vx/8;
		int vx8vy = vx8*vy;

		int x,y,z;

		loop ( x , int(pos1.x), int(pos2.x) )
		loop ( y , int(pos1.y), int(pos2.y) )
		loop ( z , int(pos1.z), int(pos2.z) )
		{	
			int bit = 1<<(x&7);
			int ofs = x/8 + y*vx8 + z * vx8vy;
			voxel[ofs]|=bit;
		}
	}
	/*------------------------------------------------------*/
	void branch( vec3f a, vec3f b , float thicknessA, float thicknessB  , float iA=0 , float iB=1)
	{
		float length = (a-b).length();

		if ( length < 1 ) return;

		vec3f rnd( getRnd() , getRnd() , getRnd());
		rnd = rnd / rnd.length();

		vec3f middle = (a+b)/2 + rnd*length*0.1f;
		float thicknessM = (thicknessA+thicknessB)/2;
		float iM = (iA+iB)/2;

		sphere(middle, thicknessM);

		vec3f add;
		add.x = float( sin ( iM * 2 * 2 * 3.1415)*thicknessM );
		add.y = 0;
		add.z = float( cos ( iM * 2 * 2 * 3.1415)*thicknessM );

		int c=color;
		color=3;
		sphere(middle+add, thicknessM/3);
		color=c;

		branch (a,middle , thicknessA,thicknessM, iA, iM );
		branch (middle,b , thicknessM,thicknessB, iM, iB );
	}
	/*------------------------------------------------------*/
	void tree( vec3f root1 ,vec3f root2 , float thickness )
	{
		static int iteration=0;
		//printf("Tree iteration %d     \r",iteration);

		if (( iteration > 5 ) || ( thickness < 2 ))
		{
			int c=color;
			color=2;
			sphere(root1, 8);
			color=c;
			return;
		}

		iteration++;
		  
		float thicknessNew = thickness * 0.4f;

		branch ( root1, root2 , thickness , thicknessNew) ;

		vec3f delta = root2 - root1;
		float length = delta.length() * 0.5f;

		for ( int branches = 0; branches < 4; branches ++)
		{
			vec3f rnd( getRnd() , getRnd() , getRnd()); getRnd() ;
			rnd = rnd / rnd.length() ;
			//rnd.y=2;
			rnd.y+=float(0.6f*iteration-0.5f);
			vec3f branch = rnd ;
			branch = branch * length / branch.length() + delta*0.6;
			tree( root2 , root2 + branch , thicknessNew );
		}

		iteration--;
	}
	/*------------------------------------------------------*/
	void loadPLY(char* filename){

		FILE* fn;

		if(filename==NULL)		::exit(0);
		if((char)filename[0]==0)	::exit(0);

		if ((fn = fopen(filename, "rb")) == NULL) ::exit(0);

		char line[1000];

		int vertices=0,faces=0;

		while(
			( fgets( line, 1000, fn ) != NULL )&& 
			( strncmp(line,"end_header",strlen("end_header")) ) 
			)
		{
			sscanf(line,"element vertex %d",&vertices);
			sscanf(line,"element face %d",&faces);
		}

		printf("read elements done\n");

		float x_min=10000000;
		float y_min=10000000;
		float z_min=10000000;
		float x_max=-10000000;
		float y_max=-10000000;
		float z_max=-10000000;

		for(int i=0;i<vertices;i++)
		{
			float x,y,z;
			fgets( line, 1000, fn );
			//if(sscanf(line,"%f %f %f",&z,&x,&y)==3)lucy
			if(sscanf(line,"%f %f %f",&z,&x,&y)==3)
			{
				vertex_arr.push_back(x);
				vertex_arr.push_back(y);
				vertex_arr.push_back(z);

				if(x<x_min)x_min=x;
				if(y<y_min)y_min=y;
				if(z<z_min)z_min=z;

				if(x>x_max)x_max=x;
				if(y>y_max)y_max=y;
				if(z>z_max)z_max=z;
			};
		}
/*		printf("xmin %f\n",x_min);
		printf("ymin %f\n",y_min);
		printf("zmin %f\n",z_min);
		printf("xmax %f\n",x_max);
		printf("ymax %f\n",y_max);
		printf("zmax %f\n",z_max);*/
		printf("read vertex_arr done\n");
		//while(1);;
					  
		

		float dx=x_max-x_min;
		float dy=y_max-y_min;
		float dz=z_max-z_min;

		printf("dx %f\n",dx);
		printf("dy %f\n",dy);
		printf("dz %f\n",dz);

		float scale = dy;
		if (dy>scale)scale=dy;
		if (dz>scale)scale=dz;

		//float scalex = dx;
		//float scaley = dy;
		//float scalez = dz;

		
			
		

		//int dir=0;


		//if(dy>dx){dx=dy;}
		///if(dz>dx){dx=dz;}

		/*
		if(dir==0)
		{
			dy=dz=dx;
			dy*=(float(vy)/float(vx));
			dz*=(float(vz)/float(vx));
		}
		if(dir==1)
		{
			dx=dz=dy;
			dx*=(float(vx)/float(vy));
			dz*=(float(vz)/float(vy));
		}
		if(dir==2)
		{
			dx=dy=dz;
			dx*=(float(vx)/float(vz));
			dy*=(float(vy)/float(vz));
		}
		*/

		//*(float(vz)/float(vy))

		for(int i=0;i<vertex_arr.size();)
		{
			vertex_arr[i]=    (vertex_arr[i]-x_min)/scale; i++;
			vertex_arr[i]=1.0-(vertex_arr[i]-y_min)/scale; i++;
			vertex_arr[i]=    (vertex_arr[i]-z_min)/scale; i++;
		};
		printf("rescale done\n");

		for(int i=0;i<faces;i++)
		{
			int n,v1,v2,v3;
			fgets( line, 1000, fn );
			if(sscanf(line,"%d %d %d %d",&n,&v1,&v2,&v3)==4)
			{
				//static int reduce=0;reduce++;
				//if((reduce%10)==0)
				{
					faces_arr.push_back(v1);
					faces_arr.push_back(v2);
					faces_arr.push_back(v3);
				}
			}
		}
		fclose(fn);
		printf("faces_arr done\n");

		/*

		int floor_id1=vertex_arr.size()/3;
		vertex_arr.push_back(0);
		vertex_arr.push_back(0.999);
		vertex_arr.push_back(0);

		int floor_id2=vertex_arr.size()/3;
		vertex_arr.push_back(1);
		vertex_arr.push_back(0.999);
		vertex_arr.push_back(0);

		int floor_id3=vertex_arr.size()/3;
		vertex_arr.push_back(1);
		vertex_arr.push_back(0.999);
		vertex_arr.push_back(1);

		int floor_id4=vertex_arr.size()/3;
		vertex_arr.push_back(0);
		vertex_arr.push_back(0.999);
		vertex_arr.push_back(1);

		faces_arr.push_back(floor_id2);
		faces_arr.push_back(floor_id1);
		faces_arr.push_back(floor_id3);
		faces++;

		faces_arr.push_back(floor_id4);
		faces_arr.push_back(floor_id3);
		faces_arr.push_back(floor_id1);
		faces++;

		printf("floor done\n");
		*/

		normal_arr.resize( vertex_arr.size() );
		for (int i=0;i<normal_arr.size();i++)
		{
			normal_arr[i]=0;
		}

		faces=faces_arr.size()/3;

		for(int i=0;i<faces;i++)
		{
			int v1 = faces_arr[i*3+0];
			int v2 = faces_arr[i*3+1];
			int v3 = faces_arr[i*3+2];

			if(v1>vertex_arr.size()-1)v1=vertex_arr.size()-1;
			if(v2>vertex_arr.size()-1)v2=vertex_arr.size()-1;
			if(v3>vertex_arr.size()-1)v3=vertex_arr.size()-1;

			vec3f p1(	vertex_arr[v1*3+0],
						vertex_arr[v1*3+1],
						vertex_arr[v1*3+2]	);

			vec3f p2(	vertex_arr[v2*3+0],
						vertex_arr[v2*3+1],
						vertex_arr[v2*3+2]	);

			vec3f p3(	vertex_arr[v3*3+0],
						vertex_arr[v3*3+1],
						vertex_arr[v3*3+2]	);

			vec3f d1=p2-p1;
			vec3f d2=p3-p1;

			vec3f   normal;
			normal= normal.cross(d1*10.0f,d2*10.0f);
			//normal.normalize();
			float len=normal.length();	if(len>0) normal= normal*(1.0f/len);

			vec3f   normal1=normal;
			vec3f   normal2=normal;
			vec3f   normal3=normal;

			normal1.x +=normal_arr[v1*3+0];
			normal1.y +=normal_arr[v1*3+1];
			normal1.z +=normal_arr[v1*3+2];
			len=normal1.length();	if(len>0) normal1= normal1*(1.0f/len);
			normal_arr[v1*3+0]=normal1.x;
			normal_arr[v1*3+1]=normal1.y;
			normal_arr[v1*3+2]=normal1.z;

			normal2.x +=normal_arr[v2*3+0];
			normal2.y +=normal_arr[v2*3+1];
			normal2.z +=normal_arr[v2*3+2];
			len=normal2.length();	if(len>0) normal2= normal2*(1.0f/len);
			normal_arr[v2*3+0]=normal2.x;
			normal_arr[v2*3+1]=normal2.y;
			normal_arr[v2*3+2]=normal2.z;

			normal3.x +=normal_arr[v3*3+0];
			normal3.y +=normal_arr[v3*3+1];
			normal3.z +=normal_arr[v3*3+2];
			len=normal3.length();	if(len>0) normal3= normal3*(1.0f/len);
			normal_arr[v3*3+0]=normal3.x;
			normal_arr[v3*3+1]=normal3.y;
			normal_arr[v3*3+2]=normal3.z;
		}
		printf("normal_arr done\n");
		//while(1);;

		printf("xmin %f\n",x_min);
		printf("ymin %f\n",y_min);
		printf("zmin %f\n",z_min);
		printf("xmax %f\n",x_max);
		printf("ymax %f\n",y_max);
		printf("zmax %f\n",z_max);
		printf("vertex_arr %d\n",vertex_arr.size());
		printf("faces_arr %d\n",faces_arr.size());

		/*
		for(int t=0;t<100;t++)
		printf("%f %f %f\n",triangle_list[t*3],triangle_list[t*3+1],triangle_list[t*3+2]);
		*/

		for(int t=0;t<faces_arr.size()/3;t++)
		{
			if(((t/9)&8191)==0)printf("Triangle %d     \r",t);
			drawPLY(t);
		}
	}
	/*------------------------------------------------------*/
	inline void drawPLY(int t,bool colorizemap=false)
	{
		int v1,v2,v3;
		v1=faces_arr[t*3+0];
		v2=faces_arr[t*3+1];
		v3=faces_arr[t*3+2];

		vec3f p1(	vertex_arr[v1*3+0],
					vertex_arr[v1*3+1],
					vertex_arr[v1*3+2]	);

		vec3f p2(	vertex_arr[v2*3+0],
					vertex_arr[v2*3+1],
					vertex_arr[v2*3+2]	);

		vec3f p3(	vertex_arr[v3*3+0],
					vertex_arr[v3*3+1],
					vertex_arr[v3*3+2]	);

		vec3f n1(	normal_arr[v1*3+0],
					normal_arr[v1*3+1],
					normal_arr[v1*3+2]	);

		vec3f n2(	normal_arr[v2*3+0],
					normal_arr[v2*3+1],
					normal_arr[v2*3+2]	);

		vec3f n3(	normal_arr[v3*3+0],
					normal_arr[v3*3+1],
					normal_arr[v3*3+2]	);

		vec3f d1=p2-p1;
		vec3f d2=p3-p1;

		vec3f dn1=n2-n1;
		vec3f dn2=n3-n1;

		float accuracy=2; 
		if(vx<1024)accuracy=3;
		if(vx<512) accuracy=4;

		int vm = (vx > vy) ? vx : vy;
		if(vz>vm)vm=vz;

		int c  = d1.length()*float(vm)*accuracy;
		int c2 = d2.length()*float(vm)*accuracy;
		if (c2>c ) c=c2;

		vec3f n,p;
	
		for(int i=0;i<=c;i++)
		{
			float a=float(i)/float(c);

			vec3f x1=p1+d1*a;
			vec3f x2=p1+d2*a;
			vec3f dx=x2-x1;

			vec3f xn1=n1+dn1*a;
			vec3f xn2=n1+dn2*a;
			vec3f dnx=xn2-xn1;

			int d = dx.length()*float(vm)*accuracy;
			
			for(int j=0;j<=d;j++)
			{
				float b=float(j)/float(d);
				p=x1+dx*b;
				if (!colorizemap)	set_voxel(p);
				else{	n=xn1+dnx*b;map_voxel(p,n);}
			}
		}
	}
	/*------------------------------------------------------*/
	inline void set_voxel(vec3f& p)
	{
		int px=p.x*vx;
		int py=p.y*vy;
		int pz=p.z*vz;

		if (px>vx-1)return;
		if (py>vy-1)return;
		if (pz>vz-1)return;
		if (px<0)return;
		if (py<0)return;
		if (pz<0)return;

		int vx8 = vx/8;
		int vx8vy = vx8*vy;
		int bit = 1<<(px&7);
		int ofs = (px>>3) + py*vx8 + pz * vx8vy;

		voxel[ofs]|=bit;		// mode 0 : add

		if(!voxel_col1)return;

		voxel_col1[ofs]&=255-bit;
		voxel_col2[ofs]&=255-bit;
		bit = px&7;
		voxel_col1[ofs]|=(color&1)<<bit;		// mode 0 : add
		voxel_col2[ofs]|=(color>>1)<<bit;		// mode 0 : add
	}
	/*------------------------------------------------------*/
	Map4 map4;
	/*------------------------------------------------------*/
	inline void map_voxel(vec3f& pos,vec3f& col)
	{
			int x=pos.x*vx;
			int y=pos.y*vy;
			int z=pos.z*vz;

			if (x>vx-1)return;
			if (y>vy-1)return;
			if (z>vz-1)return;
			if (x<0)return;
			if (y<0)return;
			if (z<0)return;

			ushort *p = map4.slabs + map4.map[x+z*map4.sx];

			uint y1=0,y2=0;

			int len1 = *p; ++p;
			int len2 = *p; ++p;

			ushort *pt = p+len1;

			int texture = 0;

			for (int s=0;s<len1;s++)
			{
				ushort slab = *p; ++p;
				
				y1+=slab&1023;
				y2=y1+(slab>>10);
				
				if(y2> y)
				if(y1<=y)
				{
					//int r_ = float(col.y);if(r_<0)r_=0;if(r_>255)r_=255;
					//pt[texture+y-y1]=r_;
					
					uint r_ = float(col.x*127+128);if(r_<0)r_=0;if(r_>255)r_=255;
					uint g_ = float(col.y*127+128);if(g_<0)g_=0;if(g_>255)g_=255;
					uint b_ = float(col.z*127+128);if(b_<0)b_=0;if(b_>255)b_=255;
					pt[texture+y-y1]=uint(r_+(b_<<8));//+(b_<<10)
					
					return;
				}
				texture+=y2-y1;
				y1=y2;
			}
	}
	/*------------------------------------------------------*/
	void colorize_map(Map4 m)
	{
		map4=m;

		for(int t=0;t<faces_arr.size()/3;t++)
		{
			if(((t/9)&8191)==0)printf("Triangle %d     \r",t);
			drawPLY(t,true);
		}
	}
	/*------------------------------------------------------*/
};


