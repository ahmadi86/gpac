#ifndef GF_OVR_RG_SC_SPH
#define GF_OVR_RG_SC_SPH

#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <GL/glew.h>

#include "ovr_util.h"

#define SPHERE_PI (float)(3.14159265359)
#define SPHERE_2PI (float)(6.28318530718)


static const char* GF_OVR_SPHERE_TEXTURE = "c:\\shaders\\image.bmp";
static const char* GF_OVR_SPHERE_VERTEX_SHADER = "c:\\shaders\\omi_tex_sphere.vert";
static const char* GF_OVR_SPHERE_FRAGMENT_SHADER = "c:\\shaders\\omi_tex_sphere.frag";


typedef struct SphereVertex_ {
	ovrVector3f position;
	ovrVector2f texCoord;
} SphereVertex;


typedef struct SphereScene_
{

	SphereVertex* sphereVertices;
	GLuint* sphereIndices;

	GLuint h_vao;
	GLuint h_vboVertices;
	GLuint h_vboIndices;

	float radius;
	int indexCount;

	// handle for shader program
	GLuint h_shaderProg;

	// handle for texture object
	GLuint h_texture;

	// handles for uniform variables in shader program
	GLuint h_texUniform;
	GLuint h_projUniform;
	GLuint h_modelViewUniform;
	GLuint h_texMatUniform;
	GLuint h_srcFmtUnifrom;

	// these will be passed to vertex shader
	ovrMatrix4f model_view;
	ovrMatrix4f projection;

	//
	ovrMatrix4f texMat;

} SphereScene;


void ovr_sphere_init(SphereScene* sphere, int rings, int sectors);
void ovr_sphere_draw(SphereScene* sphere);
void ovr_sphere_load_texture(SphereScene* sphere);
GLuint ovr_sphere_load_shaders(void);



#endif
