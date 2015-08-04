//#################################################################//
#include "Bmp.h"
//#################################################################//
Bmp::Bmp()
{
	width=height=depth=0;
	data=NULL;
}
//#################################################################//
Bmp::Bmp(const char*filename)
{
	width=height=depth=0;
	data=NULL;
	load(filename);
}
//#################################################################//
Bmp::Bmp(int x,int y,int b,unsigned char*buffer)
{
	width=height=depth=0;
	data=NULL;
	set(x,y,b,buffer);
}
//#################################################################//
Bmp::~Bmp()
{
	if (data) free(data);
}
//#################################################################//
bool Bmp::save(const char*filename)
{
	unsigned char bmp[58]=
			{0x42,0x4D,0x36,0x30,0,0,0,0,0,0,0x36,0,0,0,0x28,0,0,0,
	           	0x40,0,0,0, // X-Size
	           	0x40,0,0,0, // Y-Size
                   	1,0,0x18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	bmp[18]	=width;
	bmp[19]	=width>>8;
	bmp[22]	=height;
	bmp[23]	=height>>8;
	bmp[28]	=bpp;

	FILE* fn;
	if ((fn = fopen (filename,"wb")) != NULL)
	{
		fwrite(bmp ,1,54   ,fn);
		fwrite(data,1,width*height*(bpp/8),fn);
		fclose(fn);
		return true;
	}
	return false;
}
//#################################################################//
bool  Bmp::addalpha(unsigned char r,unsigned char g,unsigned char b)
{
  if(bpp==32)return true;

	unsigned char *data32=(unsigned char*)malloc(width*height*4);
	int x,y;
		for(x=0;x<width;x++)
			for(y=0;y<height;y++)
			{
				data32[(y*width+x)*4+0]=data[(y*width+x)*3+0];
				data32[(y*width+x)*4+1]=data[(y*width+x)*3+1];
				data32[(y*width+x)*4+2]=data[(y*width+x)*3+2];
				data32[(y*width+x)*4+3]=255;
				if(abs(r-data[(y*width+x)*3+0])<20)
				if(abs(g-data[(y*width+x)*3+1])<20)
				if(abs(b-data[(y*width+x)*3+2])<20)
				{
					data32[(y*width+x)*4+0]=0;
					data32[(y*width+x)*4+1]=0;
					data32[(y*width+x)*4+2]=0;
					data32[(y*width+x)*4+3]=0;
				}
			}
	free(data);
	data=data32;               
	bpp=32;
	
	return true;
}
//#################################################################//
bool  Bmp::normalize()
{
	int x,y,tmp;
	int bytes=bpp/8;
	int val0,val1,val2;

	if(bytes<3)return false;

	for(x=0;x<width;x++)
	for(y=0;y<height;y++)
	{
		tmp=0;
		val0=data[(y*width+x)*bytes+0];
		val1=255-val0;
		val2=data[(y*width+x)*bytes+2];
		val0=val0*(255-val2)/255;
		val1=val1*(255-val2)/255;

		data[(y*width+x)*bytes+0]=val0;
		data[(y*width+x)*bytes+1]=val1;
		data[(y*width+x)*bytes+2]=val2;
	}
	
	return true;
}
//#################################################################//
vec3f Bmp::get_f_fdx_fdy(float x,float y)
{
	float delta = 1.0f / 256.0f;
	//float zero = getPixel(0,0).x;
	float fx = (getPixel(x,y).x);///zero;//-getPixel(0,0).x
	float dx1 = ((getPixel(x+delta,y)-getPixel(x,y)).x);///(zero*4*delta);
	float dy1 = ((getPixel(x,y+delta)-getPixel(x,y)).x);///(zero*4*delta);
	float dx2 = ((getPixel(x,y)-getPixel(x-delta,y)).x);///(zero*4*delta);
	float dy2 = ((getPixel(x,y)-getPixel(x,y-delta)).x);///(zero*4*delta);

	return vec3f(fx,(dx1+dx2)/2,(dy1+dy2)/2);

	/*
	vec3f normal;	 
	vec3f tan   = vec3f( delta , dx , 0); 
	vec3f cotan = vec3f( dy , delta , 0);
	normal.cross(tan,cotan);

	normal.normalize();
	tan.normalize();
	cotan.normalize();

	matrix44 mat44;

	mat44.m[0][0] = 1;//normal.x;
	mat44.m[0][1] = 0;//normal.y;
	mat44.m[0][2] = 0;//normal.z;
	mat44.m[0][3] = 0;
	mat44.m[1][0] = 0;//cotan.x;
	mat44.m[1][1] = 1;//cotan.y;
	mat44.m[1][2] = 0;//cotan.z;
	mat44.m[1][3] = 0;
	mat44.m[2][0] = 0;//tan.x;
	mat44.m[2][1] = 0;//tan.y;
	mat44.m[2][2] = 1;//tan.z;
	mat44.m[2][3] = 0;
	mat44.m[3][0] = 0;
	mat44.m[3][1] = pos.x;
	mat44.m[3][2] = 0;
	mat44.m[3][3] = 1;

	mat44.m[0][0] = tan.x;//normal.x;
	mat44.m[0][1] = tan.y;//normal.y;
	mat44.m[0][2] = 0;//normal.z;
	mat44.m[0][3] = 0;
	mat44.m[1][0] = cotan.x;
	mat44.m[1][1] = cotan.y;
	mat44.m[1][2] = 0;//cotan.z;
	mat44.m[1][3] = 0;
	mat44.m[2][0] = normal.x;//tan.x;
	mat44.m[2][1] = normal.y;//tan.y;
	mat44.m[2][2] = normal.z;//tan.z;
	mat44.m[2][3] = 0;
	mat44.m[3][0] = pos.x/zero+1;
	mat44.m[3][1] = pos.x/zero+1;
	mat44.m[3][2] = pos.x/zero+1;
	mat44.m[3][3] = 1;

	return mat44;
	*/
}
	//#################################################################//
// scale x scale y translate
vec3f Bmp::getSxSyT(float x)
{
	bool find_r=true; int y_r = 1;
	bool find_g=true; int y_g = 1;
	bool find_b=true; int y_b = 1;

	int ix = int(x*float(width));

	for (int y = height-1;y>=0;y--)
	{
		int ofs = (y*width+ix)*(bpp/8);
		bool b = (data[ofs+0]>128) ? true : false;
		bool g = (data[ofs+1]>128) ? true : false;
		bool r = (data[ofs+2]>128) ? true : false;

		if (find_r) if (!r)
		{
			find_r = false;
			y_r = y;
		}
		if (find_g) if (!g)
		{
			find_g = false;
			y_g = y;
		}
		if (find_b) if (!b)
		{
			find_b = false;
			y_b = y;
		}
	}
	return vec3f (
			float(y_r)/float(height),
			float(y_g)/float(height),
			float(y_b)/float(height)
		);
}
//#################################################################//
vec3f Bmp::getPixel(float x,float y)
{
	if(x<0)x=0;
	if(y<0)y=0;
	if(x>1)x=1;
	if(y>1)y=1;
  int ofs = (int(y*float(height-1))*width+int(x*float(width-1)) )*(bpp/8);
  float b = float(data[ofs+0])/255.0f;
  float g = float(data[ofs+1])/255.0f;
  float r = float(data[ofs+2])/255.0f;
  return vec3f(r,g,b);
}
//#################################################################//
inline int Bmp::sampleByte(int x,int y)
{
  return (data[(  ((height-1-y)%height)*width+x%width )*(bpp/8)]);
}
//#################################################################//
inline int Bmp::sampleMap(int x,int y)
{
  return (255-data[(  (y%height)*width+x%width )*(bpp/8)]);
}
//#################################################################//
bool Bmp::normalMap(void)
{
	unsigned char* tmpData=(unsigned char*)malloc(width*height*3);

	for(int y=0;y<height;y++)
	for(int x=0;x<width;x++)
	{
		int h0=sampleMap(x, y);
		int hs=sampleMap(x-3, y);
		int ht=sampleMap(x, y-3);

		vec3f vs(10,0,float(hs-h0));
		vec3f vt(0,10,float(ht-h0));
		vec3f n;
		n.cross(vs,vt);
		n.normalize();

		tmpData[(x+y*width)*3+0]=(unsigned char)((float)(n.x*100+128));
		tmpData[(x+y*width)*3+1]=(unsigned char)((float)(n.y*100+128));
		tmpData[(x+y*width)*3+2]=(unsigned char)((float)(n.z*100+128));
	}

	free(data);

	bpp=24;
	data=tmpData;

	return true;
}
//#################################################################//
bool  Bmp::blur(int count)
{
	int x,y,b,c;
	int bytes=bpp/8;
	for(c=0;c<count;c++)
		for(x=0;x<width-1;x++)
			for(y=0;y<height-1;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+0)*width+x+1)*bytes+b]+
					      (int)data[((y+1)*width+x+0)*bytes+b]+
					      (int)data[((y+1)*width+x+1)*bytes+b] ) /4;

	return true;
}
//#################################################################//
bool  Bmp::hblur(int count)
{
	int x,y,b,a;
	int bytes=bpp/8;

	for(a=0;a<count;a++)
		for(x=0;x<width-1;x++)
			for(y=0;y<height;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+0)*width+x+1)*bytes+b]
					    ) /2;

	return true;
}
//#################################################################//
bool  Bmp::vblur(int count)
{
	int x,y,b,a;
	int bytes=bpp/8;

	for(a=0;a<count;a++)
		for(x=0;x<width;x++)
			for(y=0;y<height-1;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+1)*width+x+0)*bytes+b]
					    ) /2;

	return true;
}
//#################################################################//
void Bmp::crop(int x,int y)
{
	if(data==NULL)return;

	unsigned char* newdata;
	int i,j;

	int bytes=bpp/8;

	newdata=(unsigned char*)malloc(x*y*bytes);

	memset(newdata,0,x*y*bytes);

	for(i=0;i<y;i++)
		if(i<height)
			for(j=0;j<x*bytes;j++)
				if(j<width*bytes)
					newdata[i*x*bytes+j]=data[i*width*bytes+j];
	free(data);
	data=NULL;
	set(x,y,bpp,newdata);
}
//#################################################################//
bool Bmp::scale(int x,int y)
{
	if(data==NULL)return false;
	if(x==0)return false;
	if(y==0)return false;

	unsigned char* newdata;
	int i,j,k;

	int bytes=bpp/8;
	newdata=(unsigned char*)malloc(x*y*bytes);
	memset(newdata,0,x*y*bytes);

	for(i=0;i<y;i++)
		for(j=0;j<x;j++)
			for(k=0;k<bytes;k++)
				newdata[i*x*bytes+j*bytes+k]=data[(i*height/y)*(width*bytes)+(j*width/x)*bytes+k];

	free(data);
	data=NULL;
	set(x,y,bpp,newdata);
	return true;
}
//#################################################################//
bool Bmp::set(int x,int y,int b,unsigned char*buffer)
{
	width=x;
	height=y;
	bpp=b;
	if(data) free(data);
	data=buffer;
	if(data==NULL)
	{
		data=(unsigned char*) malloc(width*height*(bpp/8));
		memset(data,0,width*height*(bpp/8));
	}

	bmp[18]	=width;
	bmp[19]	=width>>8;
	bmp[22]	=height;
	bmp[23]	=height>>8;
	bmp[28]	=bpp;

	return true;
}
//#################################################################//
bool Bmp::set3d(int x,int y,int z,int b,unsigned char*buffer)
{
	width=x;
	height=y;
	depth=z;
	bpp=b;
	if(data) free(data);
	data=buffer;
	if(data==NULL)
	{
		data=(unsigned char*) malloc(width*height*depth*(bpp/8));
		memset(data,0,width*height*depth*(bpp/8));
	}

	bmp[18]	=width;
	bmp[19]	=width>>8;
	bmp[22]	=height;
	bmp[23]	=height>>8;
	bmp[28]	=bpp;

	return true;
}
//#################################################################//
bool Bmp::load(const char *filename,bool checktransparency,int check_r,int check_g, int check_b)
{
	FILE* handle;

	if(filename==NULL)		
		{printf("File not found %s !\n",filename);while(1);;}
	if((char)filename[0]==0)	
		{printf("File not found %s !\n",filename);while(1);;}

	if ((handle = fopen(filename, "rb")) == NULL)
		{printf("File not found %s !\n",filename);while(1);;}
		
	if(!fread(bmp, 11, 1, handle))
	{
		printf("Error reading file %s!\n",filename);
		return false;
	}
	if(!fread(&bmp[11], (int)((unsigned char)bmp[10])-11, 1, handle))
	{
		printf("Error reading file %s!\n",filename);
		return false;
	}

	width		=(int)((unsigned char)bmp[18])+((int)((unsigned char)(bmp[19]))<<8);
	height	=(int)((unsigned char)bmp[22])+((int)((unsigned char)(bmp[23]))<<8);
	bpp		=bmp[28];

	if(width*height>2048*2048)
	{
		printf("Error reading file %s - too big!\n",filename);
		fclose(handle);
		return false;
	}

	if(data)free(data);
//	if(data_tmp)free(data_tmp);
//	if(lens_lookup)free(lens_lookup);

//	data_tmp=(unsigned char*)malloc(width*height*(bpp/8));
//	memset(data_tmp,0,width*height*(bpp/8));

	switch (bpp)
	{
	case 24:
		short i,r,g,b,alpha;
		alpha=false;
		data=(unsigned char*)malloc(width*height*4);
		fread(data,width*height*3,1,handle);
		for(i=width*height-1;i>=0;i--)
		{
			b=data[i*3+0];
			g=data[i*3+1];
			r=data[i*3+2];
			if((r==check_r)&&(g==check_g)&&(b==check_b))
				alpha=true;
		}

		if(!checktransparency) break;

		if (alpha)
		{
			bpp=32;
			for(i=width*height-1;i>=0;i--)
			{
				b=data[i*4+0]=data[i*3+0];
				g=data[i*4+1]=data[i*3+1];
				r=data[i*4+2]=data[i*3+2];
				data[i*4+3]=255;
				if((r==check_r)&&(g==check_g)&&(b==check_b))
				{
					data[i*4+0]=0;
					data[i*4+1]=0;
					data[i*4+2]=0;
					data[i*4+3]=0;
				}
			}
		}
		break;

	case 32:
		data=(unsigned char*)malloc(width*height*4);
		fread(data,width*height*4,1,handle);
		break;

	default:
		printf("unknown Format - %dx%dx%d Bit",width,height,bpp);
		fclose(handle);
		return false;
	}

	fclose(handle);
	printf("read successfully %s ; %dx%dx%d Bit \n",filename,width,height,bpp);

	return true;
}

//#################################################################//
