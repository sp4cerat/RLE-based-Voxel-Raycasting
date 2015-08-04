#pragma once////////////////////////////////////////////////////////////////////////////////
//#define MEM_TEXTURE
#define SCREEN_SIZE_X 1024
#define SCREEN_SIZE_Y 768
#define RENDER_SIZE 1024
#define RAYS_CASTED (SCREEN_SIZE_X*4)
#define RAYS_CASTED_RES (SCREEN_SIZE_X*4)
#define RAYS_DISTANCE 80000
#define MIP_DISTANCE (SCREEN_SIZE_X)
#define THREAD_COUNT 128

//#define ANTIALIAS
//#define WONDERLAND
//#define MOUNTAINS
//#define BONSAI
//#define VOXELSTEIN
#define BUDDHA
//#define CLIPREGION

//#define NO_ROTATION
//#define DETAIL_BENCH
//#define HEIGHT_COLOR

#define FLOATING_HORIZON
#define XFLOATING_HORIZON

// clipping
//#define CENTERSEG

//#define NORMALCLIP
//#define PERPIXELFORWARD
#define SHAREMEMCLIP


/*
#define SCREEN_SIZE_X 640
#define SCREEN_SIZE_Y 480
#define RENDER_SIZE 1024
#define RAYS_CASTED 4096
*/

/*
#define SCREEN_SIZE_X 512
#define SCREEN_SIZE_Y 512
#define RENDER_SIZE 512
#define RAYS_CASTED 2048
*/
////////////////////////////////////////////////////////////////////////////////
/*
#define SCREEN_SIZE_X 1024
#define SCREEN_SIZE_Y 768
#define RENDER_SIZE 1024
#define RAYS_CASTED 4096
*/
////////////////////////////////////////////////////////////////////////////////
#define _USE_MATH_DEFINES
#define loop(a_l,start_l,end_l) for ( a_l = start_l;a_l<end_l;++a_l )
#define loops(a_l,start_l,end_l,step_l) for ( a_l = start_l;a_l<end_l;a_l+=step_l )

#ifndef byte
#define byte unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef uchar
#define uchar unsigned char
#endif

////////////////////////////////////////////////////////////////////////////////
class Keyboard
{
	public:

	bool  key [256]; // actual
	bool  key2[256]; // before

	Keyboard(){ int a; loop(a,0,256) key[a] = key2[a]=0; }

	bool KeyDn(char a)//key down
	{
		return key[a];
	}
	bool KeyPr(char a)//pressed
	{
		return ((!key2[a]) && key[a] );
	}
	bool KeyUp(char a)//released
	{
		return ((!key[a]) && key2[a] );
	}
	void update()
	{
		int a;loop( a,0,256 ) key2[a] = key[a];
	}
};
////////////////////////////////////////////////////////////////////////////////
class Mouse
{
	public:

	bool  button[256];
	bool  button2[256];
	float mouseX,mouseY;
	float mouseDX,mouseDY;

	Mouse()
	{ 
		int a; loop(a,0,256) button[a] = button2[a]=0; 
		mouseX=mouseY=mouseDX=mouseDY= 0;
	}
	void update()
	{
		int a;loop( a,0,256 ) button2[a] = button[a];
	}
};
////////////////////////////////////////////////////////////////////////////////
class Screen
{
	public:

	int	 window_width;
	int	 window_height;
	bool fullscreen;

	float posx,posy,posz;
	float rotx,roty,rotz;
};
////////////////////////////////////////////////////////////////////////////////
extern Keyboard		keyboard;
extern Mouse		mouse;
extern Screen		screen;
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif 
////////////////////////////////////////////////////////////////////////////////
extern void	cpu_memcpy(void* dst, void* src, int count);
extern void	gpu_memcpy(void* dst, void* src, int count);
extern void*	gpu_malloc(int size);
extern int		cpu_to_gpu_delta;
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif 
////////////////////////////////////////////////////////////////////////////////
