uniform sampler2D texDecal;
uniform sampler2D texDecal2;
varying vec2 texCoord;
varying vec3 vertex;

void main(void)
{
   	vec4  col = texture2D(texDecal,texCoord);//* 0.4 + texture2D(texDecal2,texCoord) * 0.6; 
   
	// 1/z is stored in w
	float radmin=col.w;
	float radmax=col.w;
	float radavg=col.w;

	// Seek min/max/avg depth
	for( float a=0; a < 3.1415*2.0 ; a+= 3.1415*1.9/6.0 )
	{
		vec2 coord= vec2( sin(a)*0.005 , cos(a)*0.005 )+texCoord;
		float w=texture2D(texDecal,coord).w;
		radmax=max(radmax , w);
		radmin=min(radmin , w);
		radavg=radavg+w;
	}	
	radavg=radavg*(1.0/4.0);

	// Run basic Box-filter

	float rad=0.0023*radmax;
	float rad_cur=0.0023*(col.w);

	if(rad>0.00008) // Clip depending on the depth
	{
	  vec4 colavg =col;//vec4(0.0,0.0,0.0,0.0);
	  vec4 colavg2=col;//vec4(0.0,0.0,0.0,0.0);
	  float numtodiv =1.0;
	  float numtodiv2=1.0;

	   for( float a=-1.0; a < 1.0 ; a+= 2.0/5.0 )
	   for( float b=-1.0; b < 1.0 ; b+= 2.0/5.0 )
	   {
			vec2 coord= vec2( a , b );
			vec4 cin = texture2D(texDecal,texCoord+coord*rad);

			if(cin.w>=radmax*0.7)
			{
				colavg = colavg + cin;

			//	if(abs(a)<=0.5)
			//	if(abs(b)<=0.5)
				{
					colavg2 = colavg2 + cin;
					numtodiv2=numtodiv2+1.0; 
				}

				// Counter for smoothing the silhouette
				numtodiv=numtodiv+1.0; 
			}
	   }
	   // Smoothen the silhouette
	   if(numtodiv>6.0) 
	   {
		   if(numtodiv<25.0) 
				colavg = colavg * (1.0/numtodiv);
		   else
			   colavg = colavg2 * (1.0/numtodiv2);
	   }
	   else
		colavg = col;//colavg2 * (1.0/numtodiv2);

	   col = colavg;//(1.0-scale) * col + scale * colavg;
	}
	
	
    gl_FragColor = col;
}