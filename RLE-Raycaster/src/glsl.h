#ifndef A_GLSL_H
#define A_GLSL_H
//-----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
//#include <GL/glut.h>// Header File For The GLUT Library 

// OpenGL Graphics includes
#include <GL/glew.h>
#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
// Sorry for Apple : unsigned int sampler is not available to you, yet...
// Let's switch to the use of PBO and glTexSubImage
#define USE_TEXSUBIMAGE2D
#else
#include <GL/freeglut.h>
#endif



#include <vector>
//-----------------------------------------------------------------------------

class  glShaderObject
{
   friend class glShader;

public:
   glShaderObject();
   virtual ~glShaderObject();
     
   int load(char* filename);                      //!< read file, if result is 0 everything is ok. -1: File not found, -2: Empty File, -3: no memory
   void loadFromMemory(const char* program);      //!< load program from char array, make sure program is 0 terminated! 
    
   bool compile(void);                            //!< compile program
   char* getCompilerLog(void);                    //!< get compiler messages
   GLint getAttribLocation(char* attribName);     //!< returns Attribute Location. Specify attribute name.  
 
protected:

    int                 program_type;             //!< 1=Vertex Program, 2=Fragment Program, 0=none

    GLuint              ShaderObject;             //!< Program Object
    GLubyte*            ShaderSource;             //!< ASCII Source-Code
    
    GLcharARB*          compiler_log;
    
    bool                is_compiled;              //!< true if compiled
    bool                _memalloc;                //!< true if shader allocated memory
};

//-----------------------------------------------------------------------------

class  aVertexShader : public glShaderObject
{
public:
   aVertexShader();
   ~aVertexShader(); 
};

//-----------------------------------------------------------------------------

class  aFragmentShader : public glShaderObject
{
public:
   aFragmentShader();
   ~aFragmentShader();
 
};

//-----------------------------------------------------------------------------

class  glShader
{
public:
   glShader();                  
   virtual ~glShader();
   void addShader(glShaderObject* ShaderProgram); //!< add a Vertex or Fragment Program
   
   bool link(void);                               //!< Link all Shaders
   char* getLinkerLog(void);                      //!< get Linker messages

   void begin();	                                //!< use Shader. OpenGL calls will go through shader.
   void end();		                                //!< Stop using this shader. OpenGL calls will go through regular pipeline.
   
   bool enabled;

   // Uniform Variables
   bool setUniform1f(char* varname, GLfloat v0);  //!< set float uniform to program
   bool setUniform2f(char* varname, GLfloat v0, GLfloat v1); //!< set vec2 uniform to program
   bool setUniform3f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2); //!< set vec3 uniform to program
   bool setUniform4f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); //!< set vec4 uniform to program   

   bool setUniform1i(char* varname, GLint v0);
   bool setUniform2i(char* varname, GLint v0, GLint v1);
   bool setUniform3i(char* varname, GLint v0, GLint v1, GLint v2);
   bool setUniform4i(char* varname, GLint v0, GLint v1, GLint v2, GLint v3);

   bool setUniform1fv(char* varname, GLsizei count, GLfloat *value);
   bool setUniform2fv(char* varname, GLsizei count, GLfloat *value);
   bool setUniform3fv(char* varname, GLsizei count, GLfloat *value);
   bool setUniform4fv(char* varname, GLsizei count, GLfloat *value);
   bool setUniform1iv(char* varname, GLsizei count, GLint *value);
   bool setUniform2iv(char* varname, GLsizei count, GLint *value);
   bool setUniform3iv(char* varname, GLsizei count, GLint *value);
   bool setUniform4iv(char* varname, GLsizei count, GLint *value);
   
   bool setUniformMatrix2fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);
   bool setUniformMatrix3fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);
   bool setUniformMatrix4fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value);

   // Receive Uniform variables:
   void GetUniformfv(char* name, GLfloat* values);
   void GetUniformiv(char* name, GLint* values); 

   // Vertex Attributes
   bool setVertexAttrib1f(GLuint index, GLfloat v0);
   bool setVertexAttrib2f(GLuint index, GLfloat v0, GLfloat v1);
   bool setVertexAttrib3f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
   bool setVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

   void manageMemory(void){_mM = true;}
   
	void enable(void)
	{
	   _noshader = true;
	}
	void disable(void)
	{
	   _noshader = false;
	} 
       
private:
   GLint GetUniLoc(const GLcharARB *name);        // get location of a variable
   
   GLuint              ProgramObject;             // GLProgramObject
   

   GLcharARB*          linker_log;
   bool                is_linked;
   std::vector<glShaderObject*> ShaderList;       // List of all Shader Programs

   bool                _mM;
   bool                _noshader;
};

//-----------------------------------------------------------------------------
// To simplify the process loading/compiling/linking shaders I created this
// high level interface to setup a vertex/fragment shader.
//
// Example:
//   /*LOAD:*/
//   glShaderManager SM;
//   glShader* shader = SM.loadfromFile("test.vert","test.frag");
//   if (shader==0) cout << "Error Loading, compiling or linking shader\n";
//   /*DRAW:*/
//   shader->begin();
//   shader->setUniform1f("MyFloat", 1.123);
//     glutDrawSolidSphere(1.0);
//   shader->end();
//

class  glShaderManager
{
public:
    glShaderManager();
    virtual ~glShaderManager();

    glShader* loadfromFile(char* vertexFile, char* fragmentFile);    // load vertex/fragment shader from file
    glShader* loadfromMemory(const char* vertexMem, const char* fragmentMem);
       
    bool free(glShader* o);

private:
    std::vector<glShader*>  _shaderObjectList;

};

#endif // A_GLSL_H