#pragma comment(lib,"cutil32.lib")
#pragma comment(lib,"shrUtils32.lib")
////////////////////////////////////////////////////////////////////////////////
#include "cutil.h"
#include "glsl.h"
#include "bitmap_fonts.h"
////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#pragma warning(disable:4996)
#endif
#include "../src.BestFitMem/bmalloc.h"
////////////////////////////////////////////////////////////////////////////////
// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
////////////////////////////////////////////////////////////////////////////////
// includes, GL
//#include <GL/glut.h>
float global_x;
float global_y;
float global_z;


#ifndef CUT_CHECK_ERROR_GL
void CUT_CHECK_ERROR_GL(){};
#endif
////////////////////////////////////////////////////////////////////////////////
#include "RLE4.h"
#include "RayMap.h"
#include "Tree.h"
#include "GL_Main.h"
#include "DrawUtils.h"
#include "../src.BestFitMem/bmalloc.h"
////////////////////////////////////////////////////////////////////////////////
//extern "C" RayMap_GPU ray_map_GPU;
////////////////////////////////////////////////////////////////////////////////
// declaration, forward
extern "C" void cuda_main_render2( int pbo_out, int width, int height,RayMap_GPU* raymap);
// rendering callbacks
void display();
///////////////////////////////////////////
void compute_ray_map();
///////////////////////////////////////////
static glShaderManager shader_manager;
static int rndtex=-1;
static GLuint tex_screen=-2;
static GLuint pbo_dest=-2;
static  int cuda_time = 0;
///////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv) {

	timeBeginPeriod(1); 

	int pool_size=100*1024*1024;
	char* pool     = (char*)(((uint)malloc(pool_size+16)+16)&0xfffffff0);
	char* pool_gpu = (char*)(((uint)gpu_malloc(pool_size+16)+16)&0xfffffff0);
	cpu_to_gpu_delta=((int)pool_gpu - (int)pool)&0xfffffff0;

	memset  (pool , 0,pool_size);
	add_pool(pool ,   pool_size);
	printf("pool:%d\n",(int)pool);
	//realloc(pool,1*1024*1024);
	//free(pool);
	//while(1);;

	RLE4 rle4;
	Tree tree;
if(0)
{		tree.init(1024,1024,1024,false);
		tree.set_color(1);
//		tree.init(256,256,256);
//		tree.init(1024,1024,1024,false);
//		tree.loadPLY("../happy.ply");
//		tree.loadPLY("../happy.ply");
//		tree.loadPLY("/code/data/ply/bunny.ply");
//		tree.loadPLY("/code/data/ply/zip/crytek-sponza/sponza.ply");
//		tree.loadPLY("/code/data/ply/zip/lucy/lucy_ascii.ply");
//		tree.loadPLY("/code/data/ply/happy.ply/happy.ply");
//		tree.loadPLY("/code/data/ply/zip/dragon.ply");
//		tree.loadPLY("/code/data/ply/xyzrgbdragon.ply");
//		tree.loadPLY("dragon.ply");
//		tree.loadPLY("bunny.ply");

		printf("Compress.\n");
		rle4.compress_all(tree);		
		rle4.save("../bunny.rle4");
//		rle4.save("spherescape_complex.rle4");

		tree.exit();
		return 0;
}


/*
	rle4.loadvxl("../untitled.vxl");
	rle4.save("voxelstein.rle4");
	return 0;
*/	
	printf("Init Tree.\n");

	printf("Creating Scene \r");

	rle4.init();


if(1)
{
//	if (!rle4.load("../voxelstein.rle4")) return 0;
//	if (!rle4.load("../bonsai.rle4")) return 0;
//	if (!rle4.load("../spheren.rle4")) return 0;
//	if (!rle4.load("../bunny.rle4")) return 0;
//	if (!rle4.load("../spheres.rle4")) return 0;
//	if (!rle4.load("../spheret.rle4")) return 0;
//	if (!rle4.load("../spherei.rle4")) return 0;
//	if (!rle4.load("../budda_low.rle4")) return 0;
//	if (!rle4.load("../check.rle4")) return 0;
#ifdef BUDDHA
//	if (!rle4.load("../san-miguel.rle4")) return 0;
//	if (!rle4.load("../bunny.rle4")) return 0;
	if (!rle4.load("../../Imrodh.rle4")) return 0;
//	if (!rle4.load("../../xyzrgbdragon.rle4")) return 0;
//	san-miguel
//	if (!rle4.load("../../san-miguel.rle4")) return 0;
//	if (!rle4.load("../../happy2k.rle4")) return 0;
//	if (!rle4.load("../sponza.rle4")) return 0;
//	if (!rle4.load("../../lucy.rle4")) return 0;
//	if (!rle4.load("../happy2.rle4")) return 0;
//	if (!rle4.load("../happy1k.rsle4")) return 0;
//	if (!rle4.load("../budda.rle4")) return 0;
//	if (!rle4.load("../../dragon1k.rle4")) return 0;
#endif

#ifdef WONDERLAND
	if (!rle4.load("../../spheres2.rle4")) return 0;
	
//  if (!rle4.load("../spherescape_complex.rle4")) return 0;
//	if (!rle4.load("../spherescape.rle4")) return 0;
//	if (!rle4.load("../spherescape2.rle4")) return 0;
#endif

//	if (!rle4.load("../dragon.rle4")) return 0;

//	MessageBoxA(0,"huhu","huhu",0);
//	return 0;
//
}else{
//		tree.load("../bucky.raw",32,32,32);
		/*
		if(!tree.load("../bunny.raw",512,361,512,16))
//		if(!tree.load("../bonsai.raw",512,182,512))
		{
			printf("file not found\n");
			while(1);;
		}  */

		tree.init(1024,2048,1024,false);
		tree.set_color(1);
//		tree.init(256,256,256);
//		tree.init(1024,1024,1024,false);
//		tree.loadPLY("../happy.ply");
		tree.loadPLY("../happy.ply");
//		tree.loadPLY("dragon.ply");
//		tree.loadPLY("bunny.ply");
//		tree.cube(vec3f(0,0,0),vec3f(1024,300,1024));
//		tree.cube(vec3f(0,200,0),vec3f(256,256,256));

#if 0
		for(int i=0;i<100;i++)
		{
			int x=(i*26256821+1235)&1023;
			int y=(i*i*17363643+1241)&255;
			int z=(i*14623468+2345)&1023;
			int d=(i*53472462+6225)&63;
			int dx=((i*64362557+2455)&31)-15;
			int dz=((i*76544357+7645)&31)-15;
			int dy=(((i*65452217+5321)&31)-15);//+(sin(float(x)*2*3.1415/1024)+sin(float(z)*2*3.1415/1024))*25-50;

			printf("Sphere %d	     \r",i);
			tree.set_color(0);
			if(x&1)tree.set_color(3);
			tree.sphere(vec3f(x,y+800,z),d*3+30,(i<50)? 0 : 1);
		}
		for(int i=0;i<4;i++)
		{
			int x=(i*23676321+7643)&1023;
			int y=(i*i*17365675+3567)&255;
			int z=(i*14623468+3845)&1023;
			int d=(i*53472367+7325)&63;
			int dx=((i*64364326+2455)&31)-15;
			int dz=((i*76544373+7645)&31)-15;
			int dy=(((i*65452217+5321)&31)-15);//+(sin(float(x)*2*3.1415/1024)+sin(float(z)*2*3.1415/1024))*25-50;
			printf("Tree %d		    \r",i);
			tree.set_color(1);
			tree.tree ( vec3f (x,900,z) , vec3f (x+dx,840-y+dy,z+dz) , d*2+10);
		}
		tree.cube(vec3f(0,900,0),vec3f(1024,1024,1024));
#endif
		/*
			tree.set_color(0);
			tree.sphere(vec3f(128,128,128),128,0);
			tree.set_color(1);
			tree.sphere(vec3f(0,128,128),128,0);
			*/
#if 0
		for(int i=0;i<510;i++)
		{
			int x=(i*26256821+1235)&1023;
			int y=(i*i*17363643+1241)&255;
			int z=(i*14623468+2345)&1023;
			int d=(i*53472462+6245)&15;
			int dx=((i*64362557+2455)&31)-15;
			int dz=((i*76544357+7645)&31)-15;
			int dy=(((i*65452217+5321)&31)-15)+(sin(float(x)*2*3.1415/1024)+sin(float(z)*2*3.1415/1024))*25-50;

			printf("Sphere %d	     \r",i);
			tree.set_color(0);
			if(x&1)tree.set_color(3);
			tree.sphere(vec3f(x,950+dy,z),d*3+30);

			if((i%84)==0)
			{
				printf("Tree %d		    \r",i);
				tree.set_color(1);
				tree.tree ( vec3f (x,950+dy,z) , vec3f (x+dx,940-y+dy,z+dz) , d*3+10);
			}
		}
#endif		
		
		//tree.cube(vec3f(0,900,0),vec3f(1024,1024,1024));
		//tree.cube(vec3f(0,100,0),vec3f(100,200,100));
		
  		/*
		tree.init(256,256,256);
		printf("Tree Sphere.\n");
		tree.set_color(0);
		tree.sphere(vec3f(120,120,120),80);
		tree.set_color(1);
		tree.sphere(vec3f(0,120,120),80);
		tree.set_color(2);
		tree.sphere(vec3f(120,120,0),80);
		*/
		
		printf("Compress.\n");
		rle4.compress_all(tree);		
		rle4.save("../budda_2k.rle4");
//		rle4.save("spherescape_complex.rle4");

		tree.exit();
		return 0;
	}

	
	/*
	ushort* vxl = rle4.uncompress(rle4.map4);
	int m4sx = rle4.map4.sx/2;
	int m4sy = rle4.map4.sy/2;
	int m4sz = rle4.map4.sz/2;
	rle4.clear();
	Map4 m4 = rle4.compress_mip(vxl,m4sx,m4sy,m4sz);
	rle4.map4=m4;*/

	printf("loading ready.\n");
	rle4.all_to_gpu();

	#ifdef MEM_TEXTURE
	rle4.all_to_gpu_tex();
	#endif

	printf("copy to gpu ready.\n");
	memcpy(ray_map.map4_gpu,rle4.mapgpu,10*sizeof(Map4) );
	ray_map.nummaps = rle4.nummaps;
	
	printf("ready.\n");
	
	screen.posx = 14447;
	screen.posz = 15021;
	screen.posy = -600;//won
//	screen.posy = 237;//spheres
//	screen.posy = -329;//bons
//	screen.posz = 5221;
//	screen.posy = 0   ;//bun
///	screen.posy = -1000   ;
	screen.posy = -438;//won

	//vox
#ifdef VOXELSTEIN	
	screen.posx = 0*4414;
	screen.posy = -185;
	screen.posz = 0*4924;

	screen.posx = 338;
	screen.posy = -191;
	screen.posz = 109;
#endif	
		
	//budda
/*	
	screen.posx = 1116;
	screen.posy = -470;
	screen.posz = 615;
*/
	/*
	screen.posx = 14447;
	screen.posz = 15021;
	screen.posy = -300;//won
	*/
	
	
#ifdef BUDDHA
	screen.posx = 499;
	screen.posy = -818;
	screen.posz = 941;
#endif
	
#ifdef WONDERLAND	
	screen.posx = 13522;
	screen.posz = 15892;
	screen.posy = -68;//won
#endif

#ifdef  CLIPREGION
	screen.posx = 0;
	screen.posz = 0;
#endif

#ifdef WONDERLAND	
	screen.posx = 13522;
	screen.posz = 15892;
	screen.posy = -68;//won
	screen.posy = -619;//won
#endif

#ifdef CLIPREGION
	screen.posx = -632;
	screen.posz = 512;
#endif
#ifndef  CLIPREGION
	screen.posx = 10000;
	screen.posz = 10000;
#endif


	gl_main.Init(SCREEN_SIZE_X,SCREEN_SIZE_Y,false,display);
	printf("Starting..\n");


	// start rendering mainloop
/*
	Bmp noise(256,256,24,0);
	int a,b;uchar* img=noise.data;
	loop(a,0,256)
	loop(b,0,256)
	{
		vec3f n;
		n.x = float(int(rand()&255))-127.5;
		n.y = float(int(rand()&255))-127.5;
		n.z = float(int(rand()&255))-127.5;
		n.normalize();
		n = n * 127.5;

		*img++ = n.x+127.5;
		*img++ = n.y+127.5;
		*img++ = n.z+127.5;
	}
	noise.save("noise.bmp");
	rndtex = gl_main.LoadTexBmp(noise);
	rndtex = gl_main.LoadTex("noise.bmp");
	*/

	//Bmp noise(2048,2048,24,0);
	//noise.save("noise.bmp");
	//rndtex = gl_main.LoadTexBmp(noise);
	//rndtex = gl_main.LoadTex("noise.bmp");

    glutMainLoop();


	return 0;
}
////////////////////////////////////////////////////////////////////////////////
void update_viewpoint()
{
	//int a;loop ( a,0,128 ) if ( keyboard.KeyDn(a) ) printf("\nk:%d\n",a);

	static int time1=0,time2=timeGetTime(),delta=0; 
	time1 = time2; time2 = timeGetTime(); delta=time2-time1;

	static float multiplier = 0.125f;
	float step = float(delta) * multiplier;

	screen.rotx = screen.rotx *0.9+0.1*((mouse.mouseY-0.5)*10+0.01);
	screen.roty = screen.roty *0.9+0.1*((mouse.mouseX-0.5)*10+0.01+M_PI/2);

#ifdef NO_ROTATION
	screen.rotx = 0.01;
	screen.roty = 0.01+M_PI/2;
#endif

	if(screen.rotx> M_PI-0.01)screen.rotx= M_PI-0.01;
	if(screen.rotx<-M_PI+0.01)screen.rotx=-M_PI+0.01;

	////////////////////// Direction matrix
		
	matrix44 m;
	m.ident();
	m.rotate_z(-screen.rotz );
	m.rotate_x(-screen.rotx );
	m.rotate_y(-screen.roty );

	////////////////////// Transform direction vector

	static vec3f pos( screen.posx,screen.posy,screen.posz );
	vec3f forward = m * vec3f(0,0,-step).v3();
	vec3f side	  = m * vec3f(-step,0,0).v3();
	vec3f updown  = m * vec3f(0,-step,0).v3();

	if ( keyboard.KeyDn(119) ) pos = pos + forward;
	if ( keyboard.KeyDn(115) ) pos = pos - forward;
	if ( keyboard.KeyDn(97 ) ) pos = pos + side;
	if ( keyboard.KeyDn(100) ) pos = pos - side;
	if ( keyboard.KeyDn(113) ) pos = pos + updown;
	if ( keyboard.KeyDn(101) ) pos = pos - updown;
	if ( keyboard.KeyPr(102) ) gl_main.ToggleFullscreen();
	if ( keyboard.KeyPr(43 ) ) multiplier *=2;
	if ( keyboard.KeyPr(45 ) ) if(multiplier>0.01) multiplier /=2;

	screen.posx = screen.posx*0.9+0.1*pos.x;
	screen.posy = screen.posy*0.9+0.1*pos.y;
	screen.posz = screen.posz*0.9+0.1*pos.z;

	////////////////////// Some light

	static unsigned int starttime=timeGetTime();
	//lightx = sin( float(timeGetTime()-starttime)/500 )*1000;
	//lighty = 1000;
	//lightz = cos( float(timeGetTime()-starttime)/500 )*1000;
}
////////////////////////////////////////////////////////////////////////////////
void render_to_pbo()
{
	// pbo variables
	// (offscreen) render target

	static int render_width  = RENDER_SIZE;
	static int render_height = RENDER_SIZE;

	unsigned int t1 = timeGetTime();

	if (pbo_dest == -2)
	{
		// create pbo
		gl_main.createPBO( &pbo_dest , RENDER_SIZE, RAYS_CASTED,32);//render_width,render_height );
		// create texture for blitting onto the screen
		//gl_main.createFloatTexture( &tex_screen, render_width,render_height );
		gl_main.createTexture( &tex_screen,  RENDER_SIZE, RAYS_CASTED,32);//render_width,render_height );
	}

	// run the Cuda kernel
	cuda_main_render2( pbo_dest, render_width, render_height , &ray_map );

	unsigned int t2 = timeGetTime();
//	printf("all %d fps:%2.2f    \r",
//		t2-t1,1000.0f/float(t2-t1));
	printf("\r");

    // blit convolved texture onto the screen

    // download texture from PBO
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, pbo_dest);
    glBindTexture( GL_TEXTURE_2D, tex_screen);
	
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 
		     //render_width, render_height, 
			 RENDER_SIZE, RAYS_CASTED,//GL_LUMINANCE , GL_FLOAT,NULL);//
			GL_RGBA, GL_UNSIGNED_BYTE,NULL); //BGRA GL_UNSIGNED_SHORT_5_5_5_1,NULL);//
			
    glBindTexture( GL_TEXTURE_2D, 0);
    //glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);

}
////////////////////////////////////////////////////////////////////////////////
void display_pbo()
{
    glBindBuffer( GL_PIXEL_PACK_BUFFER_ARB, 0);
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	static glShader* shader_soft=0;
	static glShader* shader_colorize=0;

	glFlush();

	if(!shader_soft)
	{
		shader_soft = shader_manager.loadfromFile(
		"shader/soft.vert",
		"shader/soft.frag");
	}
	
	if(!shader_colorize)
	{
		shader_colorize = shader_manager.loadfromFile(

#ifdef ANTIALIAS
	#ifdef VOXELSTEIN
			"shader/colorize_stein_soft_2xAA.vert",
			"shader/colorize_stein_soft_2xAA.frag");
	#endif
	#ifdef WONDERLAND
			"shader/colorize_soft_2xAA.vert",
			"shader/colorize_soft_2xAA.frag");
	#endif
	#ifdef BUDDHA
			"shader/colorize_buddha_soft_2xAA.vert",
			"shader/colorize_buddha_soft_2xAA.frag");
	#endif
#else
	#ifdef VOXELSTEIN
			"shader/colorize_stein_soft.vert",
			"shader/colorize_stein_soft.frag");
	#endif
	#ifdef WONDERLAND
			"shader/colorize_soft.vert",
			"shader/colorize_soft.frag");
	#endif
	#ifdef BUDDHA
			"shader/colorize_buddha_soft.vert",
			"shader/colorize_buddha_soft.frag");
	#endif
#endif

	}

	static FBO fbo1(2048,2048);
//	static FBO fbo2(2048,2048);
//	static int flipflop=1;
//	flipflop^=1;

//	FBO *fbo_arr[2]={&fbo1,&fbo2};
	FBO *fbo      =  &fbo1;//fbo_arr[flipflop];
//	FBO *fbo_back =  fbo_arr[flipflop^1];

	fbo->enable();
    glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

//	glActiveTextureARB( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_screen);

    // render a screen sized quad
    glDisable(GL_LIGHTING);
//	glActiveTextureARB( GL_TEXTURE0 );
    glEnable(GL_TEXTURE_2D);
    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glMatrixMode( GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();

	float border = ray_map.border;

	//glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
//    glEnable(GL_DEPTH_WRITEMASK);

	shader_colorize->begin();

	// Shader Parameters
	shader_colorize->setUniform1i("texDecal",0);	
	shader_colorize->setUniform2f("vanish",
		 1-ray_map.vanishing_point_2d.x,
		(1-ray_map.vanishing_point_2d.y-border)*
		 float(screen.window_width)/
		 float(screen.window_height)
		);

	float ofs1 = 4*float(ray_map.res[0])/float(RAYS_CASTED_RES);
	float ofs2 = 4*float(ray_map.res[1])/float(RAYS_CASTED_RES)+ofs1;
	float ofs3 = 4*float(ray_map.res[2])/float(RAYS_CASTED_RES)+ofs2;

	shader_colorize->setUniform1f("rot_x_greater_zero", (screen.rotx > 0) ? 1 : 0 );

	shader_colorize->setUniform4f("ofs_add",
		-ray_map.p_ofs_min[0],
		-ray_map.p_ofs_min[1]+ofs1,
		-ray_map.p_ofs_min[2]+ofs2,
		-ray_map.p_ofs_min[3]+ofs3		
		);
	shader_colorize->setUniform4f("res_x_y_ray_ratio",
		screen.window_width,
		screen.window_height,
		RAYS_CASTED,
		float(RAYS_CASTED_RES) / float(RAYS_CASTED)
	);

	float vright= 2.0*float(screen.window_width)  / 2048.0 - 1.0;
	float vdown = 2.0*float(screen.window_height) / 2048.0 - 1.0;
	float tright= 1.0*float(screen.window_width)  / 2048.0;
	float tdown = 1.0*float(screen.window_height) / 2048.0;

	
    glDisable( GL_BLEND);
    glBegin( GL_QUADS);
	glColor4f(1,1,1,1);
	glVertex3f( -1.0   , -1.0, 0.5);//glTexCoord2f( 0, 0);
	glVertex3f(  vright, -1.0, 0.5);//glTexCoord2f( 1, 0);
	glVertex3f(  vright,  vdown, 0.5);//glTexCoord2f( 1, 1);
	glVertex3f( -1.0   ,  vdown, 0.5);//glTexCoord2f( 0, 1);
    glEnd();
	

	shader_colorize->end();

	fbo->disable();

    glBindBuffer( GL_PIXEL_PACK_BUFFER_ARB, 0);
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, screen.window_width, screen.window_height);

	glActiveTextureARB( GL_TEXTURE1 );
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rndtex);

	glActiveTextureARB( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, fbo->color_tex);

	shader_soft->begin();
	shader_soft->setUniform1i("texDecal",0);	
//	shader_soft->setUniform1i("texDecal2",1);	
    glDisable( GL_BLEND);
    glBegin( GL_QUADS);
	glColor4f(1,1,1,1);
	glTexCoord2f( 0, 0);			glVertex3f( -1.0,-1.0, 1.0);
	glTexCoord2f( tright, 0);		glVertex3f(  1.0,-1.0, 1.0);
	glTexCoord2f( tright, tdown);	glVertex3f(  1.0, 1.0, 1.0);
	glTexCoord2f( 0, tdown);		glVertex3f( -1.0, 1.0, 1.0);
//	glTexCoord2f( 0, 0);			glVertex3f( -1.0,-1.0, 1.0);
//	glTexCoord2f( 1, 0);			glVertex3f(  1.0,-1.0, 1.0);
//	glTexCoord2f( 1, 1-border*2);	glVertex3f(  1.0, 1.0, 1.0);
//	glTexCoord2f( 0, 1-border*2);	glVertex3f( -1.0, 1.0, 1.0);
    glEnd();
	shader_soft->end();

	glActiveTextureARB( GL_TEXTURE1 );
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTextureARB( GL_TEXTURE0 );
//	glBindTexture(GL_TEXTURE_2D, rndtex);
//	glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0,0,2048,2048,0);

	glMatrixMode( GL_PROJECTION);
    glPopMatrix();

    glDisable( GL_TEXTURE_2D);

    CUT_CHECK_ERROR_GL();
	glFlush();
}
////////////////////////////////////////////////////////////////////////////////
void display_ray_map()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix
	gluPerspective(45.0f,(GLfloat)512/(GLfloat)512,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
	glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
/*
	static bool loadmodel=false;
	if (!loadmodel)
	{
		loadmodel=true;
		loadPLY("C:/Documents and Settings/alwaysWorking/Desktop/largemodels/happy.ply");
	}
	static vec3f rot(0,-180,0);
	static vec3f pos(-2.4,-2,6.6);
	
    glLoadIdentity();            
    glRotatef(rot.x,1,0,0);        
    glRotatef(rot.y,0,1,0);
    glTranslatef(pos.x,pos.y,pos.z);

    glBegin( GL_TRIANGLES);
	glColor4f(1,1,1,1);

	glVertex3f( 0,0,0);
	glVertex3f( 0,0.1,0);
	glVertex3f( 0.1,0.1,0);
	
	for(int t=0;t<triangle_list.size();)
	{
		glVertex3f( 
		triangle_list[t+0]*1, 
		triangle_list[t+1]*2, 
		triangle_list[t+2]*1);
		t+=3;
	}
    glEnd();
*/
	return;


/*
    // View adjust
	static vec3f rot(0,-180,0);
	static vec3f pos(-2.4,-2,6.6);
	
    glLoadIdentity();            
    glRotatef(rot.x,1,0,0);        
    glRotatef(rot.y,0,1,0);
    glTranslatef(pos.x,pos.y,pos.z);

	static const vec3f colors[5]={			
		vec3f(0,1,1),
		vec3f(1,0,1),
		vec3f(1,1,0),
		vec3f(0,1,0)	};

	int i=0;
	
	float ys_min =(float(SCREEN_SIZE_X)-float(SCREEN_SIZE_Y))/(2*float(SCREEN_SIZE_X));
	float ys_max =(float(SCREEN_SIZE_X)-float(SCREEN_SIZE_Y))/(2*float(SCREEN_SIZE_X))+float(SCREEN_SIZE_Y)/float(SCREEN_SIZE_X);

	vec3f plist[4]=
	{
		vec3f( 0, ys_min,0),
		vec3f( 1, ys_min,0),
		vec3f( 1, ys_max,0),
		vec3f( 0, ys_max,0)
	};

	loop (i,0,4) drawutils.Line3D( plist[i] ,plist[(i+1)&3]);

	loop (i,0,ray_map.map_line_count)
	{
			if((i%15)==0)
			{
				RayMap::MapLine &l = ray_map.map_line[i]; 
				vec3f start_2d = l.start2d;
				vec3f start_3d = l.start3d * 0.5 + vec3f(-1,3,0); 
				vec3f end_2d   = l.end2d;
				vec3f end_3d   = l.end3d * 0.5 + vec3f(-1,3,0);

				start_2d.z=0;
				end_2d.z=0;

				start_2d.y*=-1;
				end_2d.y*=-1;
				start_2d.y+=1;
				end_2d.y+=1;

				drawutils.Line3D(start_2d,end_2d);
				drawutils.Line3D(start_3d,end_3d);
			}
	}
	*/
}
////////////////////////////////////////////////////////////////////////////////
void compute_ray_map()
{
	vec3f pos ( screen.posx,screen.posy,screen.posz );
	vec3f rot ( screen.rotx,screen.roty,screen.rotz );

//	ray_map.set_border(0.5*(float(SCREEN_SIZE_X)/float(SCREEN_SIZE_Y)-1.0f));
	ray_map.set_border(0.125); // todo sven
//	ray_map.set_border(0.5*(float(screen.window_width-screen.window_height)/float(screen.window_width)));
	ray_map.set_ray_limit(RAYS_CASTED_RES);
	ray_map.get_ray_map( pos, rot);
}//
////////////////////////////////////////////////////////////////////////////////
//! GLUT Display callback
////////////////////////////////////////////////////////////////////////////////
void
display() 
{
	int t1=timeGetTime();
	update_viewpoint();
	compute_ray_map();
	int t2=timeGetTime();
#ifndef DETAIL_BENCH
	printf("raymap: %d  ",t2-t1);
#endif

	render_to_pbo();	// external CUDA call
	glFlush();

	int t3=timeGetTime();

	cuda_time = t3-t2;

	printf("renA: %d  ",t3-t2);

	display_pbo();
	display_ray_map();
	glFlush();

	int t4=timeGetTime();

#ifndef DETAIL_BENCH
	printf("tex %d  ",t4-t3);
#endif

	static float msavg=1.0;
	static int t_1=timeGetTime();
	static int t_2;
	t_2=t_1; t_1=timeGetTime();

	msavg = ( msavg * 3 + float(t_1-t_2) ) / 4.0;

	
	beginRenderText( screen.window_width, screen.window_height);
    {
		char text[100];
		int HCOL= 0,VCOL = 5;
        glColor3f( 1.0f, 1.0f, 1.0f );
		sprintf(text, "Total: %3.2f msec (%3.3f fps)", msavg, (1000.0f/msavg) );
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
		sprintf(text, "CUDA Time: %d msec", cuda_time);
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
		sprintf(text, "Rays: %d RenderTarget:%d", ray_map.map_line_count,RENDER_SIZE);
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
		sprintf(text, "Screen: %dx%d", screen.window_width,screen.window_height);
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
		sprintf(text, "Pos: %d %d %d", int(ray_map.position.x),int(ray_map.position.y),int(ray_map.position.z));
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
		sprintf(text, "Rot: %2.2f %2.2f %2.2f", (ray_map.rotation.x),(ray_map.rotation.y),(ray_map.rotation.z));
		HCOL+=15;
        renderText( VCOL, HCOL, BITMAP_FONT_TYPE_HELVETICA_12, text );
		
    }
    endRenderText();
	glFlush();

	int t5=timeGetTime();
#ifndef DETAIL_BENCH
	printf("txt %d  ",t5-t4);
#endif

	glutSwapBuffers();
	glFlush();

	int t6=timeGetTime();
#ifndef DETAIL_BENCH
	printf("swp %d  ",t6-t5);
#endif

	mouse.update();
	keyboard.update();

	int t7=timeGetTime();
#ifndef DETAIL_BENCH
	printf("keyb %d  ",t7-t6);
#endif
}
////////////////////////////////////////////////////////////////////////////////
