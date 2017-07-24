#include <gpac/glfw_app.h>

void __stdcall glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	printf("glDebugCallbackHandler, %s\n", msg);
}

GLFWwindow* ovr_glfw_create_window(const ovrVector2i size, const ovrVector2i position)
{
	GLFWwindow* window = glfwCreateWindow(size.x, size.y, "glfw", NULL, NULL);
	if (!window) {
		printf("ovr_glfw_create_window, Unable to create rendering window\n");
		return NULL;
	}
	if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
		glfwSetWindowPos(window, position.x, position.y);
	}
	return window;
}

void ovr_glfw_error_callback(int error, const char* description) {

	printf("ovr_glfw_error_callback, %s\n", description);

}

void ovr_glfw_constructor(GLFWApp *gf_ovr_GLA)
{
	memset(gf_ovr_GLA, 0, sizeof(GLFWApp));

	gf_ovr_GLA->window = NULL;

	if (!glfwInit()) {
		printf("ovr_glfw_constructor, Failed to initialize GLFW\n");
	}
	else {
		printf("ovr_glfw_constructor, Successfully initialized GLFW\n");
	}

	glfwSetErrorCallback(ovr_glfw_error_callback);
}

void ovr_glfw_deconstructor(GLFWApp *gf_ovr_GLA)
{
	if (NULL != gf_ovr_GLA->window) {
		glfwDestroyWindow(gf_ovr_GLA->window);
	}
	glfwTerminate();
}

void ovr_glfw_draw(void)
{
	return;
}

void ovr_glfw_pre_create(void) {
	glfwWindowHint(GLFW_DEPTH_BITS, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
}

void ovr_glfw_post_create(GLFWApp *gf_ovr_GLA) {

	printf("ovr_glfw_post_create\n");

	glfwSetWindowUserPointer(gf_ovr_GLA->window, gf_ovr_GLA);
	glfwSetKeyCallback(gf_ovr_GLA->window, ovr_glfw_key_callback);
	glfwSetMouseButtonCallback(gf_ovr_GLA->window, ovr_glfw_mouse_button_callback);
	glfwSetWindowUserPointer(gf_ovr_GLA->window, gf_ovr_GLA->window);
	glfwMakeContextCurrent(gf_ovr_GLA->window);

	// Initialize the OpenGL bindings
	// For some reason we have to set this experminetal flag to properly
	// init GLEW if we use a core context.
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		printf("ovr_glfw_post_create, Failed to initialize GLEW\n");
		return;
	}
	glGetError();

	printf("OpenGL version supported by this platorm is %s\n", glGetString(GL_VERSION));

	if (GLEW_KHR_debug) {
		GLint v;
		glGetIntegerv(GL_CONTEXT_FLAGS, &v);
		if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
			glDebugMessageCallback(glDebugCallbackHandler, gf_ovr_GLA);
		}
	}
}

void ovr_glfw_init_gl(void) {
}

void ovr_glfw_shutdown_gl(void) {
}

void ovr_glfw_finish_frame(GLFWApp *gf_ovr_GLA) {
	glfwSwapBuffers(gf_ovr_GLA->window);
}

void ovr_glfw_destroy_window(GLFWApp *gf_ovr_GLA) {
	glfwSetKeyCallback(gf_ovr_GLA->window, NULL);
	glfwSetMouseButtonCallback(gf_ovr_GLA->window, NULL);
	glfwDestroyWindow(gf_ovr_GLA->window);
}

void ovr_glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// do nothing
	if (action != GLFW_PRESS) {
		return;
	}

	// we will only handle ESC key here
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, 1);
		return;
	}

}

void ovr_glfw_update(void)
{
	return;
}

void ovr_glfw_onMouseButton(int button, int action, int mods)
{
	// do nothing
	return;
}

void ovr_glfw_viewport(const ovrVector2i pos, const ovrVector2i size) {
	glViewport(pos.x, pos.y, size.x, size.y);
}

void ovr_glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	glfwGetWindowUserPointer(window);
	ovr_glfw_onKey(window, key, scancode, action, mods);
}

void ovr_glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	ovr_glfw_onMouseButton(button, action, mods);
}