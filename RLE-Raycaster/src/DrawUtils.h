///////////////////////////////////////////
#pragma once
///////////////////////////////////////////
#include "VecMath.h"
#include "glsl.h"
//#include <GL/glew.h>
//#include <GL/glut.h>
///////////////////////////////////////////
struct DrawUtils
{
	///////////////////////////////////////////
	void Line3D ( vec3f p1 , vec3f p2 , vec3f color=vec3f(1,1,1)){
		glBegin(GL_LINES);
		glColor3f(color.x ,color.y ,color.z );
		glVertex3f( p1.x,p1.y,p1.z );
		glVertex3f( p2.x,p2.y,p2.z );
		glEnd();
	}
	///////////////////////////////////////////
	void TexturedRect( vec3f p[4],vec3f color=vec3f(1,1,1) )
	{
		float tex_x[4]={1,0,0,1};
		float tex_y[4]={0,0,1,1};

		glBegin(GL_POLYGON);				// start drawing a polygon
		glColor3f(color.x,color.y,color.z);

		for (int i=0;i<4;i++)
		{
			glTexCoord2f( tex_x[i], tex_y[i] );
			glVertex3f(	p[i].x,p[i].y,p[i].z);
		}
		glEnd();					// we're done with the polygon
	}
	///////////////////////////////////////////
	void Rect( vec3f p[4],vec3f color=vec3f(1,1,1) )
	{
		glBegin(GL_LINE_LOOP);				// start drawing a polygon
		glColor3f(color.x,color.y,color.z);
		glVertex3f(	p[0].x,p[0].y,p[0].z);
		glVertex3f(	p[1].x,p[1].y,p[1].z);
		glVertex3f(	p[2].x,p[2].y,p[2].z);
		glVertex3f(	p[3].x,p[3].y,p[3].z);
		glEnd();					// we're done with the polygon
	}
	///////////////////////////////////////////
};
///////////////////////////////////////////
extern DrawUtils drawutils;
///////////////////////////////////////////
