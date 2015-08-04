
varying vec2 texCoord;
varying vec3 vertex;

void main(void)
{
	texCoord	= gl_MultiTexCoord0.xy;
	vertex		= gl_Vertex.xyz;
    gl_Position	= gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz,1.0);
}