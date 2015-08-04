#pragma once

#include "mathlib/matrixdefs.h"

struct Render
{
    /*-------------------- Variables    --------------------*/

	RayMap_GPU ray_map;

	int res_x,res_y;
	int *data_rgb;

	struct Perf
	{
		int elems_total;
		int elems_processed;
		int voxels_processed;
		int elems_rendered;
		int pixels;
	};
	Perf perf[RAYS_CASTED];


    /*------------------------------------------------------*/
	void set_target(  int resolution_x, int resolution_y, int* data_rgb )
	{
		this->res_x=resolution_x;
		this->res_y=resolution_y;
		this->data_rgb=data_rgb;
		if (data_rgb==0) {printf("data_rgb 0\n");while(1);}
	}
	/*------------------------------------------------------*/
	void set_raymap( RayMap_GPU* raymap )
	{
		memcpy (&ray_map,raymap,sizeof(RayMap_GPU));
	}
	/*------------------------------------------------------*/
	inline __device__ float LineScale(vec3f input, vec3f center,float clip_max,float clip_min)
	{
		float scale_x = 1;
		float scale_y = 1;

		if (center.x>1)	scale_x = (1-input.x)/(center.x-input.x);
		if (center.x<0)	scale_x = input.x/(input.x-center.x);
		if (center.y>clip_max)	scale_y = ( clip_max-input.y)/(center.y-input.y);
		if (center.y<clip_min)	scale_y = (-clip_min+input.y)/(input.y-center.y);

		float scale = (scale_x<scale_y) ? scale_x : scale_y;

		return scale;
	}
	/*------------------------------------------------------*/
	inline __device__ void ClipLine(vec3f &p1, vec3f &p2,float clip_max,float clip_min)
	{
		vec3f c1,c2; float scale; 
		
		c1=p1;c2=p2;

		scale = LineScale(p1,p2,clip_max,clip_min);	c2 = p1+(p2-p1) * scale;
		scale = LineScale(p2,p1,clip_max,clip_min);	c1 = p2+(p1-p2) * scale;

		p1 = c1;
		p2 = c2;
	}
	/*------------------------------------------------------*/
	inline __device__ vec3f MatMul ( matrix44 m, vec3f v)
	{
		return make_float3(
		m.M11*v.x + m.M21*v.y + m.M31*v.z + m.M41,
		m.M12*v.x + m.M22*v.y + m.M32*v.z + m.M42,
		m.M13*v.x + m.M23*v.y + m.M33*v.z + m.M43);
	}
	/*------------------------------------------------------*/
    inline __device__ void vec3f_rot_y( float a , vec3f& v) 
    { 
		float xx = cos ( -a ) * v.x + sin ( -a ) * v.z;
		float zz = cos ( -a ) * v.z - sin ( -a ) * v.x;
		v.x=xx;v.z=zz;
	}

	/*------------------------------------------------------*/
    inline __device__ void vec3f_normalize( vec3f& v )
    { 
		float square = v.x*v.x + v.y*v.y + v.z*v.z;
		if (square <= 0.00001f ) 
		{
			v.x=1;v.y=0;v.z=0;
			return; 
		}
		float len = 1.0 / (float)sqrt(square); 
		v.x*=len;v.y*=len;v.z*=len;
		return; 
	}
	/*------------------------------------------------------*/
	inline __device__ void render_line 
	(
		int x, 
		unsigned int *y_cache, 
		vec3f viewpos ,
		vec3f viewrot ,
		int res_x,
		int res_y,
		ushort* ofs_skip_start
	)
	{
		float res_x2 = res_x/2;
		float res_y2 = res_y/2;
		uint*  ofs_rgb_start = (uint*)&data_rgb[x*res_y];
		uint*  ofs_cache_start = ((uint*)ofs_skip_start)+x*res_y;

		ofs_cache_start[0]=0;
		uint ofs_cache_count = 0;
		uint ofs_cache_depth = 0;

		//RayMap_GPU::MapLine ml = ray_map.map_line[x];

		float ml_ray_x;//=ml.ray_x;
		float ml_ray_z;//=ml.ray_z;
		vec3f ml_start2d;//=ml.start2d;
		vec3f ml_end2d;//=ml.end2d;
		vec3f ml_start3d;//=ml.start3d;
		vec3f ml_end3d;//=ml.end3d;
		bool  ml_direction_y;//=ml.direction_y;

		//if(0)
		//if(x>=ray_map.res[0]/2)
		{
			int rays[4];
			rays[0]=ray_map.res[0];
			rays[1]=ray_map.res[1]+rays[0];
			rays[2]=ray_map.res[2]+rays[1];
			rays[3]=ray_map.res[3]+rays[2];

			int quadrant=0;
			if (x>=rays[2]) quadrant=3;else
			if (x>=rays[1]) quadrant=2;else
			if (x>=rays[0]) quadrant=1;
			
			float quadrant_ofs = x;
			if(quadrant>=1) quadrant_ofs -= rays[quadrant-1];

			float quadrant_num = ray_map.res[quadrant];
			float a = quadrant_ofs / quadrant_num;

			int j=quadrant;

			vec3f p1,p2,p1_3d,p2_3d;
			p1 = ray_map.p_2d[5];
			p2 = ray_map.p_no[j*2]+(ray_map.p_no[j*2+1]-ray_map.p_no[j*2])*a;
			
			ClipLine(p1,p2,ray_map.clip_max,ray_map.clip_min);

			matrix44 to3d = ray_map.to3d;
			vec3f p1m4 = p1*4.0;
			vec3f p2m4 = p2*4.0;
			p1_3d = MatMul ( to3d , p1m4 );
			p2_3d = MatMul ( to3d , p2m4 );

			vec3f delta = (p1_3d+p2_3d)*0.5-ray_map.p4;
			delta.y = 0;
			delta = normalize(delta);
			vec3f_rot_y( viewrot.y , delta);

			ml_ray_x = delta.x;				
			ml_ray_z = delta.z;			
			ml_start2d=p1;
			ml_end2d=p2;
			ml_start3d=p1_3d;
			ml_end3d=p2_3d;
			ml_direction_y = 1-(j>>1);
		}

		/////////////////////////////////////////// 
		// Initialize Render Vars

		int mip_lvl=0;
		int y_clip_min	= 0;
		int y_clip_max	= res_y-1;
		const int z_far = RAYS_DISTANCE;
		float dz		= 1<<mip_lvl;
		int mapswitch	= MIP_DISTANCE;//res_y2 	;
		//if (SCREEN_SIZE_X < res_y) mapswitch	= SCREEN_SIZE_X;

		/////////////////////////////////////////// 
		// Initialize Rotation Vars

		float sin_x = sin(ray_map.rotation.x); // Rotation around x-axis
		float cos_x = cos(ray_map.rotation.x);
		float sin_y = sin(ray_map.rotation.y); // Rotation around y-axis
		float cos_y = cos(ray_map.rotation.y);

		/////////////////////////////////////////// 
		// Initialize Raymap Vars

		float ray_x = ml_ray_x;
		float ray_z = ml_ray_z;
		bool vertical = ml_direction_y;

		/////////////////////////////////////////// 
		// Reverse texturing ?

		bool reverse=false;
		if( vertical)if(ray_z<=0) reverse=true;
		if(!vertical)if(ray_x<=0)if(sin_x>0) reverse=true;
		if(!vertical)if(ray_x> 0)if(sin_x<0) reverse=true;

		float res_x2_mul_reverse   = (reverse) ? -res_x2 : res_x2;

		if(vertical)	res_x2_mul_reverse = -res_x2_mul_reverse; 

		/////////////////////////////////////////// 
		// Screenspace clipping

		int3 p1,p2;
		int p_add=(reverse)?1:-2;
		p1.x = int(float(float(res_x) * ml_start2d.x))+p_add;
		p1.y = int(float(float(res_y) * ml_start2d.y))+p_add;
		p2.x = int(float(float(res_x) * ml_end2d.x))-p_add;
		p2.y = int(float(float(res_y) * ml_end2d.y))-p_add;
		if (p1.x<0) p1.x = 0;if (p1.x>=res_x) p1.x = res_x-1;
		if (p1.y<0) p1.y = 0;if (p1.y>=res_y) p1.y = res_y-1;
		if (p2.x<0) p2.x = 0;if (p2.x>=res_x) p2.x = res_x-1;
		if (p2.y<0) p2.y = 0;if (p2.y>=res_y) p2.y = res_y-1;

		if(p1.y==p2.y) return; // If removed -> Error ..!!?? Todo

		y_clip_min = res_x-1-p1.x;
		y_clip_max = res_x-1-p2.x;

		if(vertical)
		{
			y_clip_min = res_y-1-p1.y;
			y_clip_max = res_y-1-p2.y;
		}

		if(reverse)
		{
			y_clip_min = res_y-1-y_clip_min;
			y_clip_max = res_y-1-y_clip_max;
		}

		if (y_clip_min > y_clip_max)
		{
			int tmp = y_clip_min;
			y_clip_min = y_clip_max;
			y_clip_max = tmp;
		}

		if (y_clip_min >= y_clip_max) return;

		/////////////////////////////////////////// 
		// Clear current rendered Line

		for (int n=y_clip_min;n<=y_clip_max;n++)
		{
			ofs_rgb_start[n]=0xff8844;//400000.0;//x123456*n;
			#ifdef PERPIXELFORWARD
			 ofs_skip_start[n]=0;//400000.0;//x123456*n;
			#endif			 
		}
#ifdef SHAREMEMCLIP
		for (int n=0;n<31;n++) y_cache[n]=0;
#endif			 

		/////////////////////////////////////////// 
		//
		//void RayDDS(vec3f direction,MapLine& ml)
		
		float2 direction_rot;
		direction_rot.x = ray_x * cos_y + ray_z * sin_y;
		direction_rot.y = ray_x * sin_y - ray_z * cos_y;

		float2 delta = direction_rot, frac, fix, sign;

		fix.x  = fix.y  =-1;
		frac.x = viewpos.x - int(viewpos.x);
		frac.y = viewpos.z - int(viewpos.z);
		sign.x = sign.y = -1;

		// Signs & direction for frac
		if (delta.x >= 0) { fix.x =0; sign.x = 1; frac.x = 1-frac.x; }
		if (delta.y >= 0) { fix.y =0; sign.y = 1; frac.y = 1-frac.y; }

		// Gradients    
		float2 grad0,grad1;
		grad0.y = delta.y / fabs(delta.x); grad0.x = sign.x;
		grad1.x = delta.x / fabs(delta.y); grad1.y = sign.y;

		// Intersections in x-,y- and z-plane
		float2 isect0,isect1; 
		isect0.x = grad0.x * frac.x;
		isect0.y = grad0.y * frac.x;
		isect1.x = grad1.x * frac.y;
		isect1.y = grad1.y * frac.y;

		float grad_dist0 = sqrt(grad0.x*grad0.x+grad0.y*grad0.y);
		float grad_dist1 = sqrt(grad1.x*grad1.x+grad1.y*grad1.y);
		float dds_dist0= sqrt(isect0.x*isect0.x+isect0.y*isect0.y);
		float dds_dist1= sqrt(isect1.x*isect1.x+isect1.y*isect1.y);

		float2 pos_vxl_before= {0,0}; float dds_dist_before =0;
		float2 pos_vxl		 = {0,0}; float dds_dist_now    =0; 

		int index=0,index_before=0;
		/////////////////////////////////////////// 
		//
		// Main Render Loop

		int	rle4_gridx=ray_map.map4_gpu[mip_lvl].sx;
		int	rle4_gridz=ray_map.map4_gpu[mip_lvl].sz;

		float pos3d_z_add = sin_x;
		float pos3d_y_add = (vertical) ? cos_x:0;
		  	  pos3d_y_add*=  res_x2_mul_reverse;

#ifdef DETAIL_BENCH
		int numpix = 0;
		int rndpix = y_clip_max-y_clip_min;
#endif

		uint*    map_ptr = ray_map.map4_gpu[mip_lvl].map;
		ushort* slab_ptr = ray_map.map4_gpu[mip_lvl].slabs;

		float z=0;

#ifdef CENTERSEG
		float cache_1_start	= y_clip_max;
		float cache_1_end	= y_clip_min;
#endif
		bool skipme=false;

		float y_map_switch = viewpos.y;

		mapswitch = mapswitch * (0.25*(4-abs(viewrot.x)));

		uint tex_map_ofs=0;
		
while(true){

			ofs_cache_depth++;

			while ( z>mapswitch || (y_map_switch>512.0)){

			y_map_switch = y_map_switch * 0.5;

			if (mip_lvl<ray_map.nummaps-1)
			{
				mip_lvl++;
				tex_map_ofs+=rle4_gridz;//tex_map_add;
				rle4_gridx>>=1;
				rle4_gridz>>=1;
				map_ptr = ray_map.map4_gpu[mip_lvl].map;
				slab_ptr= ray_map.map4_gpu[mip_lvl].slabs;
			}
			
			grad0.x *= 2;
			grad0.y *= 2;
			grad1.x *= 2;
			grad1.y *= 2;
			grad_dist0 *=2;
			grad_dist1 *=2;
			mapswitch *= 2;
			dz*=2;
		}
		z+=dz;
		if (z>z_far)return;

#ifndef DETAIL_BENCH
		if ((y_clip_min>=y_clip_max))	return;
//		if ((y_clip_min>>1>=y_clip_max>>1)||(numpix==rndpix))	return;
#endif

		/////////////////////////////////////////// 
		//DDA				
		dds_dist_before	= dds_dist_now;
		pos_vxl_before		= pos_vxl;
		index_before		= index;

		//dds_dist0= sqrt(isect0.x*isect0.x+isect0.y*isect0.y);
		//dds_dist1= sqrt(isect1.x*isect1.x+isect1.y*isect1.y);

		/*
		float if1 = ( dds_dist1 < dds_dist0) ? 1.0 : 0.0;
		float if0 = 1.0-if1;
		dds_dist_now = dds_dist1*if1 + dds_dist0*if0;
		index = if1;
		pos_vxl.x	  = isect1.x*if1+isect0.x*if0;
		pos_vxl.y	  = isect1.y*if1+isect0.y*if0;
		dds_dist1+=grad_dist1*if1;
		isect1.x += grad1.x*if1;
		isect1.y += grad1.y*if1;
		dds_dist0+=grad_dist0*if0;
		isect0.x += grad0.x*if0;
		isect0.y += grad0.y*if0;
		*/
		
		if ( dds_dist1 < dds_dist0)	
		{ 
			dds_dist_now = dds_dist1;
			index = 1; 
			dds_dist1+=grad_dist1;//dds_dist1; 
			pos_vxl	  = isect1  ;
			isect1.x += grad1.x ;
			isect1.y += grad1.y ;
		}else
		{
			dds_dist_now = dds_dist0;
			index = 0;
			pos_vxl	  = isect0  ;
			dds_dist0+=grad_dist0;//dds_dist1; 
			isect0.x += grad0.x ;
			isect0.y += grad0.y ;
		}

		if(skipme){skipme=false;continue;}

		int fix_x=(1-index_before)*fix.x,
			fix_z=(  index_before)*fix.y;

		/////////////////////////////////////////// 

		float dds_dist_delta = dds_dist_now-dds_dist_before;

		vec3f view_space;
		view_space.x = ray_x * dds_dist_before;
		view_space.z = ray_z * dds_dist_before;

		int voxel_x = (int( viewpos.x+pos_vxl_before.x )+fix_x);
		int voxel_z = (int( viewpos.z+pos_vxl_before.y )+fix_z);

#ifdef CLIPREGION
		if(voxel_x<0)continue;
		if(voxel_z<0)continue;
		if((voxel_x>>mip_lvl)>rle4_gridx-1)continue;
		if((voxel_z>>mip_lvl)>rle4_gridz-1)continue;
#endif				
		//if (voxel_x&1024) continue;
		//if (voxel_z&1024) continue;

		int vx = (voxel_x>>mip_lvl) & (rle4_gridx-1);
		int vz = (voxel_z>>mip_lvl) & (rle4_gridz-1);

#ifdef MOUNTAINS
		float xx1=float(voxel_x)/1000.0f;
		float zz1=float(voxel_z)/1000.0f;
//		float xx1=float(voxel_x)/1000.0f;
//		float zz1=float(voxel_z)/1000.0f;
		float sinadd =(sin(xx1/2+zz1/3)+cos(zz1/2+xx1)*0.5+cos(zz1))*500+500;
		float sinadd2=(sin(xx1/17+zz1/19)+cos(zz1/12+xx1/13)*0.5+cos(zz1/31))*4500+4500;
		float mountain = viewpos.y+sinadd;//+sinadd2;
#else
		float mountain = viewpos.y;
#endif

		float correct_x = ray_x * dds_dist_delta;
		float correct_z = ray_z * dds_dist_delta;

		float pos3d_z = cos_x*(view_space.z) + sin_x*mountain;
		float pos3d_y = (vertical) ? 
			  cos_x* mountain - sin_x*(view_space.z)
			: view_space.x;

		pos3d_y	*= res_x2_mul_reverse;

		// Clip Top //if (pos3d_z>0) 
		if (pos3d_z*res_y2 + pos3d_y <=pos3d_z*y_clip_min) continue;

#ifdef MEM_TEXTURE
		uint2 int64_ofs_rle=tex2D(
			texture_pointermap, 
			vx,vz+tex_map_ofs);
#else
		uint2 int64_ofs_rle=((uint2*)map_ptr)[vx+vz*rle4_gridx];
#endif
		uint slab_offset= int64_ofs_rle.x;
		uint len_first  = int64_ofs_rle.y; 
		ushort  slen = len_first; //if(slen==0)continue;

		float pos3d_z1,pos3d_z2;int scr_y1;
		float pos3d_y1,pos3d_y2;int scr_y2;
		
		float corr_zz= cos_x*correct_z;
		float corr_yy= (vertical) ?  -sin_x*correct_z : correct_x;

		corr_yy *= res_x2_mul_reverse;

		int sti_general = 0 ; 
		int sti_skip = 0;
		ushort sti_ ;

		sti_ = len_first >> 16;

#ifdef MEM_TEXTURE
		uint slabs = 2+slab_offset;
		uint send = slabs + slen;
#else
		ushort* slabs = slab_ptr+2+slab_offset;
		ushort* send = slabs + slen;
#endif
		float tex	 = 0;
		sti_general = 0 ; 
		sti_skip = 0;

#ifdef DETAIL_BENCH
		perf[x].elems_total+=slen;
		if ((y_clip_min>>1>=y_clip_max>>1)||(numpix==rndpix))	continue;
#endif
		uint slabs1=(uint)slabs;

		#pragma unroll 2
		for (;slabs<send;++slabs)
		{			
#ifdef MEM_TEXTURE
			if ( uint(slabs) > slabs1 )	sti_ = tex1Dfetch(texture_slabs, slabs);
#else
			if ( uint(slabs) > slabs1 )	sti_ = *slabs;
#endif

 
			sti_skip     = (sti_>>10 );	
			sti_general += (sti_&1023)<<mip_lvl;
			if (sti_skip==0)continue;

			int texture=tex;
			tex+=sti_skip;
			sti_skip <<= mip_lvl;

			float sti_general_sti_skip=sti_general;
			sti_general+=sti_skip;
		
			float correct_zz1=pos3d_z;
			float correct_yy1=pos3d_y;
			if ( mountain+sti_general_sti_skip>=0){correct_zz1+=corr_zz;correct_yy1+=corr_yy;} 

#ifdef DETAIL_BENCH
		perf[x].voxels_processed+=sti_skip;
		perf[x].elems_processed++;
#endif
			pos3d_z1=correct_zz1+pos3d_z_add*sti_general_sti_skip;if (pos3d_z1<=0) continue;
			pos3d_y1=correct_yy1+pos3d_y_add*sti_general_sti_skip;
			scr_y2 = res_y2 + pos3d_y1 / pos3d_z1 ;			
			if (scr_y2<=y_clip_min){ skipme=false;break; }

			/*
			if ( uint(slabs) == slabs1 ) // column visible
			{
				ofs_cache_count ++;
				ofs_cache_start[0]=ofs_cache_count;
				ofs_cache_start[ofs_cache_count]=ofs_cache_depth;
			}
			*/

			float correct_zz2=pos3d_z;
			float correct_yy2=pos3d_y;
			if ( mountain+sti_general <0){correct_zz2+=corr_zz;correct_yy2+=corr_yy;}

			pos3d_z2=correct_zz2+pos3d_z_add*sti_general;if (pos3d_z2<=0) continue;
			pos3d_y2=correct_yy2+pos3d_y_add*sti_general;
			scr_y1 = res_y2 + pos3d_y2 / pos3d_z2-1;if (scr_y1>=y_clip_max)	continue;
			
			
#ifndef CENTERSEG
			if (scr_y2>=y_clip_max){ 
				scr_y2=y_clip_max;
				#ifdef FLOATING_HORIZON
				y_clip_max = scr_y1;
				#endif
			}
			if (scr_y1<=y_clip_min){ 
				scr_y1=y_clip_min;
				#ifdef FLOATING_HORIZON
				y_clip_min = scr_y2;
				#endif
				#ifdef SHAREMEMCLIP
				#ifdef XFLOATING_HORIZON
				while( (y_clip_max>y_clip_min) && (y_cache[y_clip_min>>5]&(1<<(y_clip_min&31))) )++y_clip_min;
				#endif		
				#endif
			}
#else
			bool merged = false;

			if (scr_y1<y_clip_min){ if ( y_clip_min >= scr_y2 ) continue; scr_y1=y_clip_min;y_clip_min = scr_y2; merged =true; }
			if (scr_y2>y_clip_max){ if ( scr_y1 >= y_clip_max ) continue; scr_y2=y_clip_max;y_clip_max = scr_y1; merged =true; }

			/////////////////////////////////////////// 
			// Culling #2 - Check center segment

			bool y1gc1s = scr_y1>=cache_1_start;
			bool y2lc1e = scr_y2<=cache_1_end;
			
			if (y1gc1s && y2lc1e) continue;

			if (!merged) if (!y1gc1s || !y2lc1e)	
			{
				if (scr_y1<cache_1_start)
				if (scr_y2>=cache_1_start)
				if (y2lc1e)
				{
					scr_y2 = cache_1_start;
					cache_1_start=scr_y1;
				}
				if (scr_y2>cache_1_end)
				if (scr_y1<=cache_1_end)
				if (y1gc1s)
				{
					scr_y1 = cache_1_end;
					cache_1_end=scr_y2;
				}
				if (scr_y2-scr_y1 > cache_1_end-cache_1_start)
				{
					cache_1_start = scr_y1;
					cache_1_end = scr_y2;
				}
			}
#endif		
			/////////////////////////////////////////// 
			int y=scr_y1;

#ifdef PERPIXELFORWARD
	        y     +=ofs_skip_start[y];//&65535;
			//scr_y2-=ofs_skip_start[scr_y2-1]>>16;
#ifdef XFLOATING_HORIZON
			y_clip_min+=ofs_skip_start[y_clip_min];//&65535;
#endif
			if (y>=scr_y2) continue;
#endif
#ifdef NORMALCLIP
//			while( (y<scr_y2) && (((uint*)ofs_rgb_start)[y]!=0xff8844) ) ++y;
//			while( (y_clip_max>y_clip_min) && (((uint*)ofs_rgb_start)[y_clip_min]!=0xff8844)  )++y_clip_min;
#endif
#ifdef SHAREMEMCLIP
#ifdef CENTERSEG
#ifdef XFLOATING_HORIZON
			while( (y_clip_max>y_clip_min) && (y_cache[y_clip_min>>5]&(1<<(y_clip_min&31))) )++y_clip_min;
#endif
#endif
			while( (y<scr_y2) && (y_cache[y>>5]&(1<<(y&31))) )++y;
			if (y>=scr_y2) continue;
#endif
			float scr_y1r , pos3d_z2r , pos3d_y2r ;
			float scr_y2r , pos3d_z1r , pos3d_y1r ;

			pos3d_z1r=pos3d_z+pos3d_z_add*sti_general_sti_skip;
			pos3d_y1r=pos3d_y+pos3d_y_add*sti_general_sti_skip;

			pos3d_z2r=pos3d_z+pos3d_z_add*sti_general;
			pos3d_y2r=pos3d_y+pos3d_y_add*sti_general;

			scr_y2r = res_y2 + pos3d_y1r / pos3d_z1r ;			
			scr_y1r = res_y2 + pos3d_y2r / pos3d_z2r ;			

			float u1z = (tex    ) / pos3d_z2r;     
			float u2dz= (texture) / pos3d_z1r-u1z; 
			float onez1 = 1/pos3d_z2r;
			float onedz2= 1/pos3d_z1r-onez1;

			u2dz  /=scr_y2r-scr_y1r;
			onedz2/=scr_y2r-scr_y1r;
#ifdef DETAIL_BENCH
			perf[x].elems_rendered++;
#endif

#ifdef PERPIXELFORWARD
			int skip_add = ofs_skip_start[scr_y2-1];
			int iskip_add  = ofs_skip_start[scr_y1];
			int sa=0;
			//skip_add += ofs_skip_start[scr_y2-1+skip_add]&65536;
			//iskip_add += ofs_skip_start[scr_y1+iskip_add]>>16;
			//scr_y2-=ofs_skip_start[scr_y2]>>16;
			//y+=ofs_skip_start[y+skip_add];
#endif
#ifdef HEIGHT_COLOR
			int height_color = 4095-mountain+viewpos.y;
#endif

			float mult = y+1-scr_y1r;
			float uz   = u1z   + u2dz  *mult;
			float onez = onez1 + onedz2*mult;
		
#ifdef PERPIXELFORWARD
			//#pragma unroll 2
			for( ;y<scr_y2;uz+=u2dz,onez+=onedz2)
#else
			//#pragma unroll 2
			for( ;y<scr_y2;++y,uz+=u2dz,onez+=onedz2)
#endif
			{
#ifdef PERPIXELFORWARD
				int skip=ofs_skip_start[y];//&65535;

				if (skip)	{	y+=skip;continue;	}
										  
				int skip_plus  = scr_y2-1-y;//+ skip_add;
				int skip_minus =-scr_y1+y  +iskip_add;
				ofs_skip_start[y]= skip_plus ;//+ ( skip_minus<<16 );
#endif
#ifdef NORMALCLIP
				if ( ((uint*)ofs_rgb_start) [y]!=0xff8844 ) continue; 				
#endif
#ifdef SHAREMEMCLIP
				int y5  = y>>5 ; 
				int y31 = 1<<(y&31);
				if( y_cache[y5] & y31 ) continue;
#endif
				uint u = min(max(float(uz/onez),float(texture)),float(tex-1.0));
				uint real_z = int(float(1/onez))&0xfffe;

#ifdef MEM_TEXTURE
				uint color16= tex1Dfetch(texture_slabs, send+u);
#else
				uint color16=send[u];
#endif

#ifdef HEIGHT_COLOR
				ushort colorpal=color16&0xff00;
				color16=min(max( (color16&0xff)*height_color>>12 , 0 ) , 255 );
				//if(color16<0)color16=0;
				//if(color16>255)color16=255;
				color16|=colorpal;
#endif
				((uint*)ofs_rgb_start) [y]  = color16+(real_z<<16);//+(real_z<<16);//depth;//send[uint(u)]
#ifdef PERPIXELFORWARD
				++y;
#endif				
#ifdef DETAIL_BENCH
				perf[x].pixels++;
#endif
#ifdef SHAREMEMCLIP
				y_cache[y5] |= y31;
#endif				
			}// pixel loop
		}//rle loop
//} // oversampling loop
};
	};
	/*------------------------------------------------------*/
};
