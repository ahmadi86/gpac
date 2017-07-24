#ifndef OVR_GLFW_APP
#define OVR_GLFW_APP

#include<stdio.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define INT_MIN     (-2147483647 - 1)

typedef struct GLFWApp_
{
	ovrVector2i windowSize;
	ovrVector2i windowPosition;
	GLFWwindow * window;
	unsigned int frame;

} GLFWApp;


void __stdcall glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data);

GLFWwindow * ovr_glfw_create_window(const ovrVector2i size, const ovrVector2i position);
void ovr_glfw_constructor(GLFWApp *gf_ovr_GLA);
void ovr_glfw_deconstructor(GLFWApp *gf_ovr_GLA);

void ovr_glfw_draw(void);
void ovr_glfw_pre_create(void);
void ovr_glfw_post_create(GLFWApp *gf_ovr_GLA);
void ovr_glfw_init_gl(void);
void ovr_glfw_shutdown_gl(void);
void ovr_glfw_finish_frame(GLFWApp *gf_ovr_GLA);
void ovr_glfw_destroy_window(GLFWApp *gf_ovr_GLA);
void ovr_glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
void ovr_glfw_update(void);
void ovr_glfw_onMouseButton(int button, int action, int mods);
void ovr_glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ovr_glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void ovr_glfw_error_callback(int error, const char* description);

#endif
