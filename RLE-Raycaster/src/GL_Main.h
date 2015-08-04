////////////////////////////////////////////////////////////////////////////////
#pragma once
////////////////////////////////////////////////////////////////////////////////
#include "core.h"
#include "bmp.h"
#include "glsl.h"
////////////////////////////////////////////////////////////////////////////////
struct GL_Main
{
	enum TexLoadFlags {
	  TEX_ADDALPHA=1,
	  TEX_NORMALMAP=2,
	  TEX_HORIZONMAP=4,
	  TEX_HORIZONLOOKUP=8,
	  TEX_NORMALIZE=16,
	  TEX_16BIT=32,
	};

	static GL_Main *This;

	bool fullscreen;

	GL_Main() { This=(GL_Main *)this; }

	// Init
	void Init(int window_width, int window_height, bool fullscreen,void (*display_func)(void));
	void ToggleFullscreen();

	// GLUT Keyboard & Mouse IO
	static void keyDown1Static   (int key, int x, int y);
    static void keyDown2Static   (unsigned char key, int x, int y);
    static void keyUp1Static     (int key, int x, int y);
    static void keyUp2Static     (unsigned char key, int x, int y);
    void KeyPressed(int key, int x, int y,bool pressed) ;
	static void MouseMotionStatic (int x,int y);
	static void MouseButtonStatic (int button, int state, int x, int y);

	// GLUT draw functions
	static void reshape_static(int w, int h);
	static void idle_static();

	// PBO functions
	void deletePBO( GLuint* pbo);
	void createPBO( GLuint* pbo,int image_width , int image_height,int bpp=32);

	// Texture functions
	void createTexture( GLuint* tex_name, unsigned int size_x, unsigned int size_y,int bpp=32);
	void createFloatTexture( GLuint* tex_name, unsigned int size_x, unsigned int size_y);
	void deleteTexture( GLuint* tex);
	int LoadTex(const char *name,int flags=0);
	int LoadTexBmp(Bmp &bmp,int flags=0);
	void UpdateTexBmp(int handle, Bmp &bmp,int flags=0);

	static void get_error()
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) 
		{
			printf("GL Error: %s\n",gluErrorString(err));
			printf("Programm Stopped!\n");
			while(1)Sleep(1000);;
		}
	}
};

class FBO {

	public:

	enum Type { COLOR=1 , DEPTH=2 }; // Bits

	int color_tex;
	int color_bpp;
	int depth_tex;
	int depth_bpp;
	Type type;

	int width;
	int height;

	int tmp_viewport[4];

	FBO (int texWidth,int texHeight)//,Type type = Type(COLOR | DEPTH),int color_bpp=32,int depth_bpp=24)
	{
		color_tex = -1;
		depth_tex = -1;
		fbo = -1;
		dbo = -1;
		init (texWidth, texHeight);
	}

	void clear ()
	{		
		if(color_tex!=-1)
		{
			// destroy objects
			glDeleteRenderbuffersEXT(1, &dbo);
			glDeleteTextures(1, (GLuint*)&color_tex);
			glDeleteTextures(1, (GLuint*)&depth_tex);
			glDeleteFramebuffersEXT(1, &fbo);
		}
	}

	void enable()
	{
		//glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
			GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
			GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_tex, 0);

		glGetIntegerv(GL_VIEWPORT, tmp_viewport);
		glViewport(0, 0, width, height);		// Reset The Current Viewport And Perspective Transformation
		//glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
		//glLoadIdentity();
		//gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
		//glMatrixMode(GL_MODELVIEW);

	}

	void disable()
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glViewport(
			tmp_viewport[0],
			tmp_viewport[1],
			tmp_viewport[2],
			tmp_viewport[3]);
		//glMatrixMode(GL_PROJECTION);
		//glPopMatrix();
		//glMatrixMode(GL_MODELVIEW);
	}

	void init (int texWidth,int texHeight)//,Type type = Type(COLOR | DEPTH),int color_bpp=32,int depth_bpp=24)
	{
	//	clear ();
		this->width = texWidth;
		this->height = texHeight;

		glGenFramebuffersEXT(1, &fbo); 
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);    
		get_error();

			// init texture
			glGenTextures(1, (GLuint*)&color_tex);
			glBindTexture(GL_TEXTURE_2D, color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
				texWidth, texHeight, 0, 
				GL_RGBA, GL_FLOAT, NULL);

			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0,
			//	    GL_RGBA, GL_UNSIGNED_BYTE, NULL);

			//GL_TEXTURE_2D,GL_RGBA, bmp.width, bmp.height,
			//	/*GL_RGBA*/GL_BGRA_EXT, GL_UNSIGNED_BYTE, bmp.data );

			get_error();
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glFramebufferTexture2DEXT(
				GL_FRAMEBUFFER_EXT, 
				GL_COLOR_ATTACHMENT0_EXT, 
				GL_TEXTURE_2D, color_tex, 0);
			get_error();

		glGenRenderbuffersEXT(1, &dbo);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo);

			glGenTextures(1, (GLuint*)&depth_tex);
			glBindTexture(GL_TEXTURE_2D, depth_tex);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 
			//	texWidth, texHeight, 0, 
			//	GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			get_error();
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);




			glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 
				texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT/*GL_UNSIGNED_INT*/, NULL);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
								GL_TEXTURE_2D, depth_tex, 0);

			get_error();
			glBindTexture(GL_TEXTURE_2D, 0);// don't leave this texture bound or fbo (zero) will use it as src, want to use it just as dest GL_DEPTH_ATTACHMENT_EXT

		get_error();

		check_framebuffer_status();
	    
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	}

	private:

	GLuint fbo; // frame buffer object ref
	GLuint dbo; // depth buffer object ref

	void get_error()
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) 
		{
			printf("GL FBO Error: %s\n",gluErrorString(err));
			printf("Programm Stopped!\n");
			while(1);;
		}
	}

	void check_framebuffer_status()
	{
		GLenum status;
		status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		switch(status) {
			case GL_FRAMEBUFFER_COMPLETE_EXT:
				return;
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
				printf("Unsupported framebuffer format\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				printf("Framebuffer incomplete, missing attachment\n");
				break;
//			case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
//				printf("Framebuffer incomplete, duplicate attachment\n");
//				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				printf("Framebuffer incomplete, attached images must have same dimensions\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				printf("Framebuffer incomplete, attached images must have same format\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				printf("Framebuffer incomplete, missing draw buffer\n");
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				printf("Framebuffer incomplete, missing read buffer\n");
				break;
			case 0:
				printf("Not ok but trying...\n");
				return;
				break;
			default:;
				printf("Framebuffer error code %d\n",status);
				break;
		};
		printf("Programm Stopped!\n");
		while(1)Sleep(100);;
	}
};

////////////////////////////////////////////////////////////////////////////////
extern GL_Main gl_main;
////////////////////////////////////////////////////////////////////////////////
