// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <GL/glut.h>// Header File For The GLUT Library 

#define IN_CUDA_ENV

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#endif
#include <cutil.h>
#include <cuda_gl_interop.h>
#include "cutil_math.h"
////////////////////////////////////////////////////////////////////////////////
#include "RayMap.h"
#include "Cuda_Render.h"
#include "Rle4.h"
#include "../src.BestFitMem/bmalloc.h"
//#include "CudaMath.h"
////////////////////////////////////////////////////////////////////////////////

texture<uint2, 2, cudaReadModeElementType> texture_pointermap;
texture<unsigned short, 1, cudaReadModeElementType> texture_slabs;
#include "Cuda_Render.h"
////////////////////////////////////////////////////////////////////////////////

cudaArray* cu_array;
cudaChannelFormatDesc channelDesc;

void create_cuda_1d_texture(char* h_data, int size)
{
	int d_size = ((size >> 8)+1)<<8;
	printf("d_size %d size %d \n",d_size,size);
	uint *d_octree;
    CUDA_SAFE_CALL(cudaMalloc((void**) &d_octree, d_size));
    CUDA_SAFE_CALL(cudaMemcpy((void *)d_octree, (void *)h_data, size, cudaMemcpyHostToDevice) );
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(16, 0, 0, 0, cudaChannelFormatKindUnsigned);

    // set texture parameters
    texture_slabs.addressMode[0] = cudaAddressModeClamp;
    texture_slabs.addressMode[1] = cudaAddressModeClamp;
    texture_slabs.addressMode[2] = cudaAddressModeClamp;
    texture_slabs.filterMode = cudaFilterModePoint;
    texture_slabs.normalized = false;    // access with normalized texture coordinates
    CUDA_SAFE_CALL(cudaBindTexture(0, texture_slabs, d_octree, channelDesc) );
}
////////////////////////////////////////////////////////////////////////////////

cudaArray* cu_array_pointermap;
cudaChannelFormatDesc channelDesc_pointermap;

void create_cuda_2d_texture(uint* h_data, int width,int height)
{
	// Allocate CUDA array in device memory 
    channelDesc_pointermap = 
               cudaCreateChannelDesc(32, 32, 0, 0,	
			   cudaChannelFormatKindUnsigned); 
	    
    cudaMallocArray(&cu_array_pointermap, &channelDesc_pointermap, width, height); 
 
    // Copy to device memory some data located at address h_data 
    // in host memory  
    cudaMemcpyToArray(cu_array_pointermap, 0, 0, h_data, width*height*8, 
                      cudaMemcpyHostToDevice); 
 
    // Set texture parameters 
    texture_pointermap.addressMode[0] = cudaAddressModeClamp; 
    texture_pointermap.addressMode[1] = cudaAddressModeClamp; 
    texture_pointermap.addressMode[2] = cudaAddressModeClamp;
    texture_pointermap.filterMode     = cudaFilterModePoint; 
    texture_pointermap.normalized     = false; 
 
    // Bind the array to the texture 
    cudaBindTextureToArray(
		texture_pointermap, 
		cu_array_pointermap, 
		channelDesc_pointermap); 
	
	/*
	int d_size = (((size >> 8)+1)<<8);
	printf("d_size %d size %d \n",d_size,size);
	uint *d_data;

	channelDesc = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindUnsigned);//<unsigned int>();//<unsigned int>();//
	CUDA_SAFE_CALL( cudaMallocArray( &cu_array, &channelDesc, tex_w, tex_h )); 
	CUDA_SAFE_CALL( cudaMemcpyToArray( cu_array, 0, 0, (void*)(texdata) , tex_w*tex_h*4, cudaMemcpyHostToDevice));

	texture_array.addressMode[0] = cudaAddressModeWrap;
	texture_array.addressMode[1] = cudaAddressModeWrap;
    texture_array.addressMode[2] = cudaAddressModeClamp;
	texture_array.filterMode = cudaFilterModePoint;//cudaFilterModeLinear;
	texture_array.normalized = false;    // access with normalized texture coordinates

	// Bind the array to the texture
	CUDA_SAFE_CALL( cudaBindTextureToArray( texture_array, cu_array, channelDesc));
    CUDA_SAFE_CALL( cudaThreadSynchronize() );



    CUDA_SAFE_CALL(cudaMalloc((void**) &d_octree, d_size));
    CUDA_SAFE_CALL(cudaMemcpy((void *)d_octree, (void *)h_data, size, cudaMemcpyHostToDevice) );
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindUnsigned);

    // set texture parameters
    texture_array.addressMode[0] = cudaAddressModeClamp;
    texture_array.addressMode[1] = cudaAddressModeClamp;
    texture_array.filterMode = cudaFilterModePoint;
    texture_array.normalized = false;    // access with normalized texture coordinates
    CUDA_SAFE_CALL(cudaBindTexture(0, texture_array, d_octree, channelDesc) );
	*/
}
////////////////////////////////////////////////////////////////////////////////
// GL ERROR CHECK
int ChkGLError(char *file, int line)
{
	//return 0;
	return 0;
}
#define C_CHECK_GL_ERROR() ChkGLError(__FILE__, __LINE__)
////////////////////////////////////////////////////////////////////////////////
extern "C" void cuda_main_render2( int pbo_out, int width, int height,RayMap_GPU* raymap);
extern "C" void pboRegister(int pbo);
extern "C" void pboUnregister(int pbo);
int	cpu_to_gpu_delta=0;
////////////////////////////////////////////////////////////////////////////////
void gpu_memcpy(void* dst, void* src, int size)
{
	CUDA_SAFE_CALL( cudaMemcpy( dst, src, size, cudaMemcpyHostToDevice) );
	CUT_CHECK_ERROR("cudaMemcpy cudaMemcpyHostToDevice failed");
}
////////////////////////////////////////////////////////////////////////////////
void cpu_memcpy(void* dst, void* src, int size)
{
	CUDA_SAFE_CALL( cudaMemcpy( dst, src, size, cudaMemcpyDeviceToHost) );
	CUT_CHECK_ERROR("cudaMemcpy cudaMemcpyDeviceToHost failed");
}
////////////////////////////////////////////////////////////////////////////////
void* gpu_malloc(int size)
{
	void* ptr=0;	
	CUDA_SAFE_CALL( cudaMalloc( (void**) &ptr, size ) );
	CUT_CHECK_ERROR("cudaMalloc failed");
	if(ptr==0){printf("\ncudaMalloc %d MB: out of memory error\n",(size>>20));while(1);;}
	return ptr;
}
////////////////////////////////////////////////////////////////////////////////
__global__ void
cudaRender(
		   Render* render_local,
		   int maxrays, 
		   vec3f viewpos, 
		   vec3f viewrot, 
		   int res_x, 
		   int res_y,
		   ushort* skipmap_gpu
		  )
{
    extern __shared__ int sdata[];
   
    int x = ( blockIdx.y * 2 + blockIdx.x )* blockDim.x + threadIdx.x;
   
	//if(x&1)return;
    if (x>=maxrays) return;
    
    //render_local->render_line(x,(unsigned int*)&sdata[((x)&127)*31]);
    render_local->render_line
	(
		x,
		(unsigned int*)&sdata[((x)&(THREAD_COUNT-1))*(16300/(THREAD_COUNT*4))],//31
		viewpos,
		viewrot,
		res_x,
		res_y,
		skipmap_gpu+x*res_y
	);

	return;
}
////////////////////////////////////////////////////////////////////////////////
void cuda_main_render2( int pbo_out, int width, int height,RayMap_GPU* raymap)
{
	int t0 = timeGetTime();

	if(pbo_out==0) return;

    static Render render;
    static Render *render_gpu=(Render*) ((char*)bmalloc(sizeof(Render))+cpu_to_gpu_delta);
    static ushort* skipmap_gpu=(ushort*)((char*)bmalloc(RAYS_CASTED*RENDER_SIZE*4)+cpu_to_gpu_delta);
    
    if((int)render_gpu==cpu_to_gpu_delta){ printf("render_gpu 0 \n");while(1);;}
    int lines_to_raycast = raymap->map_line_count;
    int thread_calls = ((raymap->map_line_count/2) | (THREAD_COUNT-1)) +1;
    if (lines_to_raycast>RAYS_CASTED ) lines_to_raycast=RAYS_CASTED;
    int* out_data;   
    CUDA_SAFE_CALL(cudaGLMapBufferObject( (void**)&out_data, pbo_out));   
	if(out_data==0) return;

	dim3 threads(THREAD_COUNT,1,1 );
    dim3 grid( 2 , thread_calls /(threads.x),1 );

    render.set_target( width, height, (int*) out_data);
  	render.set_raymap( raymap );

#ifdef DETAIL_BENCH
	for(int t=0;t<RAYS_CASTED;t++)
	{
		render.perf[t].elems_total=0;
		render.perf[t].elems_processed=0;
		render.perf[t].voxels_processed=0;
		render.perf[t].elems_rendered=0;
		render.perf[t].pixels=0;
	}
#endif
	
	gpu_memcpy(render_gpu, &render, sizeof(Render));
   
	int t1 = timeGetTime();CUDA_SAFE_CALL( cudaThreadSynchronize() );

	//printf("before\n");
	//Sleep(10000);

	if(1)
	cudaRender<<< grid, threads, 16300 >>>
	(
		render_gpu,
		render.ray_map.map_line_count,
		render.ray_map.position,
		render.ray_map.rotation,
		render.res_x,
		render.res_y,
		skipmap_gpu
	);
	
	CUT_CHECK_ERROR("cudaRender failed");
//	CUT_CHECK_ERROR_GL();
	C_CHECK_GL_ERROR();

	CUDA_SAFE_CALL( cudaThreadSynchronize() );
	int t2 = timeGetTime();

#ifdef DETAIL_BENCH
	cpu_memcpy(&render.perf[0],&(render_gpu->perf[0]),  sizeof(Render::Perf)*RAYS_CASTED);
	Render::Perf p;
	p.elems_total=0;
	p.elems_processed=0;
	p.voxels_processed=0;
	p.elems_rendered=0;
	p.pixels=0;
	for(int t=0;t<RAYS_CASTED;t++)
	{
		p.elems_total+=render.perf[t].elems_total;
		p.elems_processed+=render.perf[t].elems_processed;
		p.voxels_processed+=render.perf[t].voxels_processed;
		p.elems_rendered+=render.perf[t].elems_rendered;
		p.pixels+=render.perf[t].pixels;
	}
	
	printf ("all %2.2fM proc %2.2fM vp %2.2fM ren %2.2fM pix %2.2fM ",
		float(p.elems_total)/(1000*1000),
		float(p.elems_processed)/(1000*1000),
		float(p.voxels_processed)/(1000*1000),
		float(p.elems_rendered)/(1000*1000),
		float(p.pixels)/(1000*1000));
#endif		
	//printf ("mem%d ren%d ",t1-t0,t2-t1);
    
    CUDA_SAFE_CALL(cudaGLUnmapBufferObject( pbo_out));
}
////////////////////////////////////////////////////////////////////////////////
void pboRegister(int pbo)
{
    // register this buffer object with CUDA
    CUDA_SAFE_CALL(cudaGLRegisterBufferObject(pbo));
	CUT_CHECK_ERROR("cudaGLRegisterBufferObject failed");
	C_CHECK_GL_ERROR();
}
////////////////////////////////////////////////////////////////////////////////
void pboUnregister(int pbo)
{
    // unregister this buffer object with CUDA
    CUDA_SAFE_CALL(cudaGLUnregisterBufferObject(pbo));	
	CUT_CHECK_ERROR("cudaGLUnregisterBufferObject failed");
	C_CHECK_GL_ERROR();
}
////////////////////////////////////////////////////////////////////////////////
/*
__global__ void
cudaColorNodes(uint* nodebuf)
{
    int x = (blockIdx.x * blockDim.x + threadIdx.x);
    int y = (blockIdx.y * blockDim.y + threadIdx.y);

	ushort* node = (ushort*)(((uint*)nodebuf) [x+y*1024]);

	uint col_rgb=0xff8844;
	if(node)
	{
		ushort col=(ushort)*node;

		const int col_r[4]={130 ,255,255,155};
		const int col_g[4]={255 ,155,0  ,255};
		const int col_b[4]={130 ,0  ,0  ,0};						

		int col_o=(col>>8)&3;				
		int bright = col&255 ;

		int r_=(bright*col_r[col_o])>>8;
		int g_=(bright*col_g[col_o])>>8;
		int b_=(bright*col_b[col_o])>>8;

		col_rgb = r_+(g_<<8)+(b_<<16) ;
	}

	((uint*)nodebuf) [x+y*1024] = col_rgb;
}
*/
////////////////////////////////////////////////////////////////////////////////
