#ifndef OVR_RIFT_APP
#define OVR_RIFT_APP

#include <stdio.h>

#include <windows.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <Windows.h>
//#include <GL/glew.h>
//#include <GL/GL.h>
//#include <GL/glext.h>

#include <GLFW/glfw3.h>
//#include <kazmath\kazmath.h>

#include "glfw_app.h"
#include "rift_manager_app.h"
#include "sphere_scene.h"
#include "ovr_util.h"


typedef struct RiftGLApp_
{
	ovrTextureSwapChain		_eyeTexture;
	ovrMirrorTexture		_mirrorTexture;
	ovrEyeRenderDesc		_eyeRenderDescs[2];
	ovrLayerEyeFov			_sceneLayer;
	ovrViewScaleDesc		_viewScaleDesc;

	ovrMatrix4f				_eyeProjections[2];
	ovrSizei				_renderTargetSize;
	ovrSizei				_mirrorSize;
	GLuint					_fbo;
	GLuint					_depthBuffer;
	GLuint					_mirrorFbo;

	GLuint					vaoID;
	GLuint					program;
	GLuint					vboID;

	// scene sphere
	SphereScene				sphere;

	// handles for video textures (each channel one texture)
	GLuint					video_texture_Y;
	GLuint					video_texture_U;
	GLuint					video_texture_V;

} RiftGLApp;


void			gf_ovr_rift_constructor(RiftGLApp* gf_ovr_RGA, GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA);
int				gf_ovr_rift_init_gl(RiftGLApp* gf_ovr_RGA, RiftManagerApp* gf_ovr_RMA);
void			gf_ovr_rift_onKey(GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA, int key, int scancode, int action, int mods);
void			gf_ovr_rift_draw(RiftGLApp* gf_ovr_RGA, GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA);
void			gf_ovr_rift_shutdown(void);
GLFWwindow*		gf_ovr_rift_create_rendering_target(ovrVector2i size, ovrVector2i pos);
int				gf_ovr_rift_run1(RiftGLApp* gf_ovr_RGA, GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA);
int				gf_ovr_rift_run2(RiftGLApp* gf_ovr_RGA, GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA);
int				gf_ovr_rift_run3(RiftGLApp* gf_ovr_RGA, GLFWApp* gf_ovr_GLA, RiftManagerApp* gf_ovr_RMA);

// scene
void			gf_ovr_rift_sc_constructor(RiftGLApp* gf_ovr_RGA);
void			gf_ovr_rift_sc_init_gl(RiftGLApp* gf_ovr_RGA, RiftManagerApp* gf_ovr_RMA);
void			gf_ovr_rift_sc_render(RiftGLApp* gf_ovr_RGA, const ovrMatrix4f projection, const ovrMatrix4f modelView);

#endif
