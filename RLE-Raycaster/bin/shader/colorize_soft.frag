#extension GL_EXT_gpu_shader4 : enable
uniform sampler2D texDecal;
uniform sampler2D texRnd;
uniform vec2 vanish;
uniform vec4 ofs_add;
uniform float rot_x_greater_zero;
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
   //Y

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

	vec4 color[6];
	color[0]=vec4(0.8,1.0,0.3,1.0);
	color[1]=vec4(1.2,0.7,0.3,1.0);
	color[2]=vec4(1.5,0.8,0.1,1.0);
	color[3]=vec4(0.2,0.8,0.2,1.0);
	color[4]=vec4(1.0,0.7,0.0,1.0);
	color[5]=vec4(0.3,0.6,1.2,1.0);

   vec4 c1= texture2D(texDecal,texpos);    
  // vec4 c2 = texture2D(texDecal,texpos+vec2( 0.0/1024.0 ,  1.0/8192.0 )); 
   //vec4 c3 = texture2D(texDecal,texpos+vec2( 1.0/1024.0 ,  0.0/8192.0 )); 
  // vec4 c4 = texture2D(texDecal,texpos+vec2( 1.0/1024.0 ,  1.0/8192.0 )); 
	int col_o1=int(c1.y*256.0)&3;				
	//int col_o2=int(c2.y*256.0)&3;				
	//int col_o3=int(c3.y*256.0)&3;				
	//int col_o4=int(c4.y*256.0)&3;				
//   if(c1.z != 1.0) c1=color[col_o1]*((c1.x-0.1)*1.1)*(c1.z*1.0+0.2)*1.1;
  // else c1=color[4]*(1.0-scy)+color[5]*(scy);
//   if(c2.z != 1.0) c2=color[col_o2]*((c2.x-0.1)*1.1)*(c2.z*1.0+0.2)*1.1;
 //  if(c3.z != 1.0) c3=color[col_o3]*((c3.x-0.0)*1.0);//*(c3.z*1.0+0.2)*1.1;
  // if(c4.z != 1.0) c4=color[col_o4]*((c4.x-0.0)*1.0);//*(c4.z*1.0+0.2)*1.1;
  
   float fragz=0;

   if(c1.z != 1.0)
   {
	   float z=1.0-(c1.w+c1.z*(1.0/256.0));
		fragz = 0.001/(1.0-z*1.0);
	  // vec4 cc= color[1]*pow(((1.0-c1.x)+0.0),4.0)*0.5;//*(c1.z*1.0+0.0)*0.5;
	   c1=color[col_o1]*c1.x*c1.x;//(color[col_o1]*pow(((c1.x-0.0)+0.0),2.0)*1.0+cc)*(z*z+0.0);
   }
   else c1=color[4]*(2.0-scy*2.0)+color[5]*(scy);
   /*
   if(c2.z != 1.0)
   {
	   float z=1.0-(c2.w+c2.z*(1.0/256.0));
		fragz = 0.001/(1.0-z*1.0);
	   vec4 cc= color[1]*pow(((1.0-c2.x)+0.0),4.0)*0.5;//*(c1.z*1.0+0.0)*0.5;
	   c2=(color[col_o2]*pow(((c2.x-0.0)+0.0),2.0)*1.0+cc)*(z*z+0.0);
   }
   else c2=color[4]*(2.0-scy*2.0)+color[5]*(scy);
   */
   
   vec4 c = c1;//(c1+c2)*0.5;//(c1+c2+c3+c4) * 0.25	  ;
 
//	if(c.z != 1.0) c=color[col_o]*((c.x-0.1)*1.1)*(c.z*1.0+0.2)*1.1;
//	if(c.z != 1.0) c=vec4(1.0,1.0,1.0,1.0)*((c.x-0.0)*1.0);//*(c.z*1.0+0.2);
//	c=c*1.1;//+vec4(0.1,0.1,0.1,0.0);
 
	c.w = fragz;
   gl_FragColor = c;//*seg_dn;
}