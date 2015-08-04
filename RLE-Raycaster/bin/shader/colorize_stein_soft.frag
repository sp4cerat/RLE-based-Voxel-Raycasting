#extension GL_EXT_gpu_shader4 : enable
uniform sampler2D texDecal;
uniform sampler2D texRnd;
uniform float rot_x_greater_zero;
uniform vec2 vanish;
uniform vec4 ofs_add;
varying vec2 texCoord;
varying vec3 vertex;

uniform vec4 res_x_y_ray_ratio;

void main(void)
{
   float RESX  = res_x_y_ray_ratio.x;//1024.0;
   float RESY  = res_x_y_ray_ratio.y;//768.0;
   float MAXRAY= res_x_y_ray_ratio.z;//2048.0;

   vec2 texpos= texCoord;
   float scx = gl_FragCoord.x/RESX;
   float scy = gl_FragCoord.y/RESY;
   
   float border=(RESX-RESY)/(RESX*2);
   
   float scx1= scx-vanish.x;
   float scy1= scy-vanish.y;

   float upper = step(scy1,0.0);
   float left  = step(scx1,0.0);
   float ostep = step(abs(scy1)-abs(scx1)*RESX/RESY,0.0);

   float seg_up =  (1-upper)*(1-ostep);
   float seg_dn =  (  upper)*(1-ostep);
   float seg_rt =  (1-left )*(  ostep);
   float seg_lt =  (  left )*(  ostep);

   float o2 = (ostep*gl_FragCoord.x+(1-ostep)*gl_FragCoord.y)/RESX;
   
   float ang2=scx1*abs(1-step(scy1,0.0)-vanish.y)/scy1+
		step(scy1,0.0)    *(1-vanish.x)+
		(1-step(scy1,0.0))*(vanish.x);
		
   float ang3=scy1*abs(1-step(scx1,0.0)-vanish.x)/scx1+
		step(scx1,0.0)    *(1-vanish.y)+
		(1-step(scx1,0.0))*(vanish.y);

   ang3= ang3*RESY/RESX+border;
   
   //X
   //texpos.y=(ofs_add.x+1.0-(ang3*ostep*RESY/RESX+ang2*(1-ostep)))*0.25; // 0
   //texpos.y=(ofs_add.y+ang3*ostep*RESY/RESX+ang2*(1-ostep))*0.25;

//   float x_pre = (ang3*ostep*RESY/RESX+ang2*(1-ostep)) ;
   float x_pre = (ostep*ang3+ang2*(1-ostep)) ;


   texpos.y=  seg_dn * ( ofs_add.y+    x_pre ) + 
	          seg_up * ( ofs_add.x+1.0-x_pre ) +
	          seg_lt * ( ofs_add.w+    x_pre ) +
	          seg_rt * ( ofs_add.z+1.0-x_pre )
			  ;

   texpos.y = texpos.y * res_x_y_ray_ratio.w * 0.25;

   //Y
//   texpos.x = (seg_dn+seg_up ) * ( o2+border )
//	   +      (seg_rt        ) * ( o2  )
//	   +      (seg_lt        ) * ( 1.0-o2 );
   float seg_up_x = rot_x_greater_zero * seg_up + (1.0-rot_x_greater_zero) * seg_dn;
   float seg_dn_x = rot_x_greater_zero * seg_dn + (1.0-rot_x_greater_zero) * seg_up;
   float seg_rt_x = rot_x_greater_zero * seg_rt + (1.0-rot_x_greater_zero) * seg_lt;
   float seg_lt_x = rot_x_greater_zero * seg_lt + (1.0-rot_x_greater_zero) * seg_rt;

   texpos.x = 
			  (seg_up_x		 ) * ( o2+border )
	   +	  (seg_dn_x		 ) * ( 1.0-(o2+border) )
	   +      (seg_rt_x      ) * ( o2  )
	   +      (seg_lt_x      ) * ( 1.0-o2 );
   
   //float cc=0.5*(step(frac(32*texpos.x),0.5)+step(frac(32*texpos.y),0.5));
   //float cc=seg_lt;
   //vec4 c = vec4(cc,cc,cc,1.0);

   vec4 c = texture2D(texDecal,texpos); 
   int x1=c.r*255.0;
   int x2=int(c.g*255.0)*256+x1;
   float col16b=float( (x2&31))/31.0;//+((x2>>5)<<(3+8))+((x2>>5)<<(3+8))
   float col16g=float(((x2>>5)&31))/31.0;//+((x2>>5)<<(3+8))+((x2>>5)<<(3+8))
   float col16r=float(((x2>>10)&31))/31.0;//+((x2>>5)<<(3+8))+((x2>>5)<<(3+8))
	
	int col_o=int(c.y*256.0)&3;				

	vec4 color[4];

	color[0]=vec4(0.5,1.0,0.0,1.0);
	color[1]=vec4(1.0,0.5,0.0,1.0);
	color[2]=vec4(1.0,0.1,0.1,1.0);
	color[3]=vec4(0.2,1.0,0.1,1.0);
	float fragz = 0.0;
	if(c.z != 1.0){
	//	c=color[col_o]*((c.x-0.1)*1.1)*(c.z*1.0+0.2);
		float z=(c.z*(1.0/256.0)+c.w);
		fragz = 0.001/(z*1.0);
		float pos3dx = z*(scx*2.0-1.0);
		float pos3dy = z*(scy*2.0-1.0);

		vec3 nrm;
		nrm.x=col16r;
		nrm.y=col16g;
		nrm.z=col16b;
		/*
		nrm = 2.0 * nrm - vec3(1.0,1.0,1.0);

		vec3 lgt;
		lgt.x=pos3dx;
		lgt.y=pos3dy;
		lgt.z=z;
		lgt.x=lgt.x-10.0;
		lgt.y=lgt.y+5.0;
		lgt=normalize(lgt);

		float light=dot(nrm.xyz,lgt.xyz);

		c.x=light;
		c.y=light;
		c.z=light;
		*/
		c.xyz=nrm.xyz;
		//c=c*(z*z-0.2);
		//c=c*vec4(0.9,0.6,0.3,1.0)+c*c*vec4(0.5,0.4,0.3,1.0);
	}
    else 
	c=vec4(0.5,0.5,0.5,1.0)*(1.0-scy)+vec4(1.0,0.5,0.0,1.0)*(scy);

	c=c*1.1;//+vec4(0.1,0.1,0.1,0.0);
	c.w = fragz;
	gl_FragColor = c;
}