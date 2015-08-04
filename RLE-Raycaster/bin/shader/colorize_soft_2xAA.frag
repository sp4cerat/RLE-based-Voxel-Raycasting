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
//    texpos.x=scy;
  //  texpos.y=scx;
   //texpos.y*=2.0;

   vec4 c_out=vec4(0.0,0.0,0.0,0.0);
   float fragz = 0.0;

   for (int i=0;i<2;i++)
   {

   vec4 c = texture2D(texDecal,texpos);//*0.5+
	        //texture2D(texDecal,texpos+vec2( 0.0/1024.0 ,  1.0/8192.0 ))*0.5;  

   if(i==1) c=texture2D(texDecal,texpos+vec2( 0.0/1024.0 ,  1.0/8192.0 ));  


	

	if(c.z != 1.0){
		float z=(c.z*(1.0/256.0)+c.w);
		fragz = 0.001/z;
		float pos3dx = z*(scx*2.0-1.0);
		float pos3dy = z*(scy*2.0-1.0);

		vec3 nrm; 
		nrm.x=c.r;//*1.0-0.0;//col16r;
		nrm.y=c.g;//*1.0-0.0;//col16g;
		//nrm.z=min(1.0,nrm.x+nrm.y);//*1.0-0.0;//col16g;
		
		nrm = 2.0 * nrm - vec3(1.0,1.0,1.0);
		nrm.z=sqrt(1.0-c.r*c.r-c.g*c.g);//col16b;
		nrm = normalize(nrm);

		vec3 lgt;
		lgt.x=pos3dx;
		lgt.y=pos3dy;
		lgt.z=z;
		lgt.x=lgt.x-30.0;
		lgt.y=lgt.y+25.0;
		lgt=normalize(lgt);

		//float light=c.g*1.6-0.3;//
		float light=(1.0-c.g)*1.0+(0.0+c.r)*0.3-0.5;;//dot(nrm.xyz,vec3(1.0,0.5,0.0));

		c.x=light;
		c.y=light;
		c.z=light;
		
		//c=c*0.5+pow(max(c,0.0),18.0)*0.4;//+vec4(0.2,0.2,0.2,0.0);
		c=c*vec4(0.9,0.5,0.3,1.0)+1.2*pow(max(c,0.0),4.0)*vec4(1.2,1.2,1.2,1.0);
		//c.y=c.x;
		//c.z=c.x;
	}
    else 
	c=vec4(178.0/255.0,204.0/255.0,1.0,1.0);;//vec4(0.3,0.3,0.3,1.0)*(1.0-scy)+vec4(0.8,0.8,0.8,1.0)*(scy);
	//c=c*1.1;//+vec4(0.1,0.1,0.1,0.0);

	c_out+=c;

   }
   c_out*=0.5;
	
  
  // c.z=c.x;
	c_out.w = fragz;
	gl_FragColor = c_out;
   //gl_FragDepth = scx;//fragz;
}