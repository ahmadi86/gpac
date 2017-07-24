#include <gpac/rift_app.h>"

void gf_ovr_rift_constructor(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	gf_ovr_RGA->_eyeTexture = NULL;

	printf("ovr_rift_constructor, call ovr_rift_manager_constructor\n");
	gf_ovr_rift_manager_constructor(gf_ovr_RMA);

	printf("ovr_rift_constructor, call ovr_glfw_constructor\n");
	ovr_glfw_constructor(gf_ovr_GLA);  // initialize GLFW

	printf("ovr_rift_constructor, call ovr_rift_sc_constructor\n");
	gf_ovr_rift_sc_constructor(gf_ovr_RGA);  // nothing much 

	gf_ovr_RGA->_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

	memset(&gf_ovr_RGA->_sceneLayer, 0, sizeof(ovrLayerEyeFov));
	gf_ovr_RGA->_sceneLayer.Header.Type = ovrLayerType_EyeFov;
	gf_ovr_RGA->_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

	// initialize render target size
	gf_ovr_RGA->_renderTargetSize.w = 0;
	gf_ovr_RGA->_renderTargetSize.h = 0;

	for (int eye = 0; eye < 2; eye++)
	{
		printf("ovr_rift_constructor, eye=%d, fov_l=%f, tex_r=%f\n", eye, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].LeftTan, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].RightTan);

		gf_ovr_RGA->_eyeRenderDescs[eye] = ovr_GetRenderDesc(gf_ovr_RMA->_session, eye, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye]);
		ovrEyeRenderDesc erd = gf_ovr_RGA->_eyeRenderDescs[eye];

		ovrMatrix4f ovrPerspectiveProjection =
			ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);

		gf_ovr_RGA->_eyeProjections[eye] = ovrPerspectiveProjection;
		gf_ovr_RGA->_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

		gf_ovr_RGA->_sceneLayer.Fov[eye] = gf_ovr_RGA->_eyeRenderDescs[eye].Fov;
		ovrFovPort fov = gf_ovr_RGA->_sceneLayer.Fov[eye];
		ovrSizei eyeSize = ovr_GetFovTextureSize(gf_ovr_RMA->_session, eye, fov, 1.0f);
		gf_ovr_RGA->_sceneLayer.Viewport[eye].Size = eyeSize;

		printf("ovr_rift_constructor, eye=%d, tex_w=%d, tex_h=%d\n", eye, eyeSize.w, eyeSize.h);

		ovrVector2i p;
		p.x = gf_ovr_RGA->_renderTargetSize.w;
		p.y = 0;

		gf_ovr_RGA->_sceneLayer.Viewport[eye].Pos = p;

		printf("ovr_rift_constructor, render target size = %d x %d\n", gf_ovr_RGA->_renderTargetSize.w, gf_ovr_RGA->_renderTargetSize.h);

		gf_ovr_RGA->_renderTargetSize.h = gf_ovr_RGA->_renderTargetSize.h >= eyeSize.h ? gf_ovr_RGA->_renderTargetSize.h : eyeSize.h;
		gf_ovr_RGA->_renderTargetSize.w += eyeSize.w;
	}


	// Make the on screen window 1/4 the resolution of the render target
	gf_ovr_RGA->_mirrorSize = gf_ovr_RGA->_renderTargetSize;
	gf_ovr_RGA->_mirrorSize.h /= 4;
	gf_ovr_RGA->_mirrorSize.w /= 4;
	printf("ovr_rift_constructor, mirror target size = %d x %d\n", gf_ovr_RGA->_mirrorSize.w, gf_ovr_RGA->_mirrorSize.h);
}

int gf_ovr_rift_init_gl(RiftGLApp *gf_ovr_RGA, RiftManagerApp *gf_ovr_RMA)
{
	printf("ovr_rift_init_gl, tex_w=%d, tex_h=%d\n", gf_ovr_RGA->_renderTargetSize.w, gf_ovr_RGA->_renderTargetSize.h);

	ovr_glfw_init_gl();  // currently does nothing

						 // disable the v-sync for buffer swap
	glfwSwapInterval(0);

	//printf("gf_ovr_RGA_init, v-sync for buffer swap disabled.\n");

	printf("ovr_rift_init_gl, ovrSession address is %p\n", (void*)gf_ovr_RMA->_session);

	ovrTextureSwapChainDesc desc;
	memset(&desc, 0, sizeof(ovrTextureSwapChainDesc));
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = gf_ovr_RGA->_renderTargetSize.w;
	desc.Height = gf_ovr_RGA->_renderTargetSize.h;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(gf_ovr_RMA->_session, &desc, &gf_ovr_RGA->_eyeTexture);
	gf_ovr_RGA->_sceneLayer.ColorTexture[0] = gf_ovr_RGA->_eyeTexture;
	if (!OVR_SUCCESS(result)) {
		printf("ovr_rift_init_gl, Failed to create swap textures (result = %d)\n", result);
		return -1;
	}

	int length = 0;
	result = ovr_GetTextureSwapChainLength(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, &length);
	if (!OVR_SUCCESS(result) || !length) {
		printf("ovr_rift_init_gl, Unable to count swap chain textures\n");
		return -1;
	}

	printf("ovr_rift_init_gl, ovr_GetTextureSwapChainLength, length=%d\n", length);

	for (int i = 0; i < length; ++i) {
		GLuint chainTexId;
		ovr_GetTextureSwapChainBufferGL(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, i, &chainTexId);

		printf("ovr_rift_init_gl, ovr_GetTextureSwapChainBufferGL, tex_id=%d\n", chainTexId);

		glBindTexture(GL_TEXTURE_2D, chainTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// Set up the framebuffer object
	glGenFramebuffers(1, &gf_ovr_RGA->_fbo);
	glGenRenderbuffers(1, &gf_ovr_RGA->_depthBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gf_ovr_RGA->_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gf_ovr_RGA->_depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, gf_ovr_RGA->_renderTargetSize.w, gf_ovr_RGA->_renderTargetSize.h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gf_ovr_RGA->_depthBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	ovrMirrorTextureDesc mirrorDesc;
	memset(&mirrorDesc, 0, sizeof(ovrMirrorTextureDesc));
	mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	mirrorDesc.Width = gf_ovr_RGA->_mirrorSize.w;
	mirrorDesc.Height = gf_ovr_RGA->_mirrorSize.h;

	printf("ovr_rift_init_gl, call ovr_CreateMirrorTextureGL\n");
	if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(gf_ovr_RMA->_session, &mirrorDesc, &gf_ovr_RGA->_mirrorTexture))) {
		printf("ovr_rift_init, Could not create mirror texture\n");
	}
	glGenFramebuffers(1, &gf_ovr_RGA->_mirrorFbo);

	gf_ovr_rift_sc_init_gl(gf_ovr_RGA, gf_ovr_RMA);

	return 0;
}

int gf_ovr_rift_run1(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	printf("ovr_rift_run1, call ovr_rift_pre_create\n");
	ovr_glfw_pre_create();  // setup OpenGL context version and window hints

	printf("ovr_rift_run1, call ovr_rift_create_rendering_target\n");
	ovrVector2i sz;
	sz.x = gf_ovr_RGA->_mirrorSize.w;
	sz.y = gf_ovr_RGA->_mirrorSize.h;

	printf("ovr_rift_run1, Mirror window size = %d x %d\n", sz.x, sz.y);

	// actually create the window
	gf_ovr_GLA->window = gf_ovr_rift_create_rendering_target(sz, gf_ovr_GLA->windowPosition);

	if (!gf_ovr_GLA->window) {
		printf("ovr_rift_run1, Unable to create OpenGL window\n");
		return -1;
	}
	else
	{
		printf("ovr_rift_run1, we have a window\n");
	}

	printf("ovr_rift_run1, call ovr_rift_post_create\n");
	ovr_glfw_post_create(gf_ovr_GLA);  // init GLEW and make context current

	printf("ovr_rift_run1, call ovr_rift_init_gl\n");
	gf_ovr_rift_init_gl(gf_ovr_RGA, gf_ovr_RMA);

	return 0;
}

int gf_ovr_rift_run2(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	//fprintf(stderr, "gf_ovr_RGA_run2, window = %d\n", gf_ovr_GLA->window);

	if (gf_ovr_GLA->window != NULL)
	{
		//fprintf(stderr, "gf_ovr_RGA_run2, window not null\n");

		while (!glfwWindowShouldClose(gf_ovr_GLA->window)) {

			//fprintf(stderr, "ovr_rift_run2, valid window\n");

			++gf_ovr_GLA->frame;

			//fprintf(stderr, "ovr_rift_run2, call glfwPollEvents, frame=%d\n", gf_ovr_GLA->frame);
			glfwPollEvents();

			//printf("ovr_rift_run2, call ovr_rift_update\n");
			ovr_glfw_update();

			//printf("ovr_rift_run2, call ovr_rift_draw\n");
			gf_ovr_rift_draw(gf_ovr_RGA, gf_ovr_GLA, gf_ovr_RMA);

			//	printf("gf_ovr_RGA_run2, call gf_ovr_GLA_finishFrame\n");
			ovr_glfw_finish_frame(gf_ovr_GLA);
		}
	}
	else
	{
		fprintf(stderr, "ovr_rift_run2, window is NULL\n");
	}
	return 0;
}


int gf_ovr_rift_run3(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	printf("ovr_rift_run3\n");

	ovr_glfw_shutdown_gl();

	return 0;
}


GLFWwindow * gf_ovr_rift_create_rendering_target(ovrVector2i size, ovrVector2i pos)
{
	return ovr_glfw_create_window(size, pos);
}


void gf_ovr_rift_onKey(GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA, int key, int scancode, int action, int mods) {

	if (GLFW_PRESS == action) switch (key) {
	case GLFW_KEY_R:
		ovr_RecenterTrackingOrigin(gf_ovr_RMA->_session);
		return;
	}

	ovr_glfw_onKey(gf_ovr_GLA->window, key, scancode, action, mods);

}

ovrMatrix4f getHeadPose(ovrPosef pose)
{
	ovrMatrix4f out = ovrMatrix4f_CreateIdentity();

	//float *quat = &pose.Orientation;

	//ovrMatrix4f orientation;
	//ovrMatrix4f translation;

	// TODO xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

	ovrMatrix4f orientation = ovrMatrix4f_CreateFromQuaternion(&pose.Orientation);
	ovrMatrix4f translation = ovrMatrix4f_CreateTranslation(pose.Position.x, pose.Position.y, pose.Position.z);

	ovrMatrix4f result = ovrMatrix4f_Multiply(&translation, &orientation);

	//mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
	//mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
	//return translation * orientation;

	return result;

}

void gf_ovr_rift_draw(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA) {

	//fprintf(stderr, "ovr_rift_draw\n");

	ovrPosef eyePoses[2];
	ovr_GetEyePoses(gf_ovr_RMA->_session, gf_ovr_GLA->frame, 1, gf_ovr_RGA->_viewScaleDesc.HmdToEyeOffset, eyePoses, &gf_ovr_RGA->_sceneLayer.SensorSampleTime);

	//fprintf(stderr, "ovr_rift_draw, pos=%d %d %d %d\n\n", eyePoses[0].Orientation.x, eyePoses[0].Orientation.y, eyePoses[0].Orientation.y, eyePoses[0].Orientation.z);

	int curIndex;
	ovr_GetTextureSwapChainCurrentIndex(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, &curIndex);
	GLuint curTexId;
	ovr_GetTextureSwapChainBufferGL(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, curIndex, &curTexId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gf_ovr_RGA->_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (int eye = 0; eye < 2; eye++)
	{
		const ovrRecti vp = gf_ovr_RGA->_sceneLayer.Viewport[eye];
		glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
		gf_ovr_RGA->_sceneLayer.RenderPose[eye] = eyePoses[eye];

		gf_ovr_rift_sc_render_scene(gf_ovr_RGA, gf_ovr_RGA->_eyeProjections[eye], getHeadPose(eyePoses[eye]));
	}

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	ovr_CommitTextureSwapChain(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture);
	ovrLayerHeader* headerList = &gf_ovr_RGA->_sceneLayer.Header;
	ovr_SubmitFrame(gf_ovr_RMA->_session, gf_ovr_GLA->frame, &gf_ovr_RGA->_viewScaleDesc, &headerList, 1);

	GLuint mirrorTextureId;
	ovr_GetMirrorTextureBufferGL(gf_ovr_RMA->_session, gf_ovr_RGA->_mirrorTexture, &mirrorTextureId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gf_ovr_RGA->_mirrorFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
	glBlitFramebuffer(0, 0, gf_ovr_RGA->_mirrorSize.w, gf_ovr_RGA->_mirrorSize.h, 0, gf_ovr_RGA->_mirrorSize.h, gf_ovr_RGA->_mirrorSize.w, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	//glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void gf_ovr_rift_shutdown(void)
{
	ovr_Shutdown();
}


//scene
void gf_ovr_rift_sc_constructor(RiftGLApp *gf_ovr_RGA)
{
	gf_ovr_RGA->program = 0;

	/*gf_ovr_RGA->earth = NULL;
	gf_ovr_RGA->earth = gluNewQuadric();
	gluQuadricTexture(gf_ovr_RGA->earth, GL_TRUE);
	gf_ovr_RGA->earthTexture = 100;
	gf_ovr_RGA->earthTexture = gf_ovr_RGA_sc_LoadBitmap("EarthTexture.bmp");

	fprintf(stderr, "gf_ovr_RGA_sc_constructor, earthTexture=%d\n", gf_ovr_RGA->earthTexture);*/


}

void gf_ovr_rift_sc_init_gl(RiftGLApp* gf_ovr_RGA, RiftManagerApp* gf_ovr_RMA) {

	fprintf(stderr, "ovr_rift_sc_init_gl\n");

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	ovr_RecenterTrackingOrigin(gf_ovr_RMA->_session);

	// generate a sphere mesh using 30 rings and 30 sectors
	ovr_sphere_init(&gf_ovr_RGA->sphere, 30, 30);
}

void gf_ovr_rift_sc_render_scene(RiftGLApp *gf_ovr_RGA, const ovrMatrix4f projection, const ovrMatrix4f headPose) {

	//fprintf(stderr, "ovr_rift_sc_render_scene\n");

	/*
	float invOut[4][4];

	if (ovr_rift_sc_invert(headPose.M, invOut) == -1)
	{
	fprintf(stderr, "ovr_rift_sc_render_scene, no inverse\n");
	return;
	}

	ovrMatrix4f *headPoseInverted = headPoseInverted = (ovrMatrix4f *)&invOut;
	*/

	//ovrMatrix4f pose = ovrMatrix4f_Transpose(headPose);

	//gf_ovr_RGA_sc_render(gf_ovr_RGA, projection, *headPoseInverted);
	ovr_sphere_draw(&gf_ovr_RGA->sphere);
}

void gf_ovr_rift_sc_render(RiftGLApp *gf_ovr_RGA, const ovrMatrix4f projection, const ovrMatrix4f headPose)
{
	fprintf(stderr, "ovr_rift_sc_render\n");

	if (gf_ovr_RGA->program != 0)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(gf_ovr_RGA->program);

		glBindVertexArray(gf_ovr_RGA->vaoID);

		glEnableVertexAttribArray(8);
		glBindBuffer(GL_ARRAY_BUFFER, gf_ovr_RGA->vboID);
		glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(8);

		glBindVertexArray(0);

		glUseProgram(0);
	}
	else
	{
		fprintf(stderr, "ovr_rift_sc_render, no program\n");
		gf_ovr_RGA->program = gf_ovr_rift_sc_load_shaders();
	}
}



GLuint gf_ovr_rift_sc_load_shaders(void)
{
	fprintf(stderr, "ovr_rift_sc_load_shaders\n");

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* rawVertShaderSource = "#version 330 core\n layout(location = 8) in vec3 in_pos;\n void main()\n {\n gl_Position.xyz = in_pos;\n gl_Position.w = 1;\n}";
	const char* rawFragShaderSource = "#version 330 core\n out vec3 color;\n void main() \n{\n color = vec3(1, 1, 1);\n }";

	glShaderSource(vertShader, 1, &rawVertShaderSource, NULL);
	glShaderSource(fragShader, 1, &rawFragShaderSource, NULL);

	glCompileShader(vertShader);
	GLint result = GL_TRUE;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "Vertex shader compilation failed..\n");
	}
	glCompileShader(fragShader);
	result = GL_TRUE;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "Fragment shader compilation failed..\n");
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	result = GL_TRUE;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "Linking failed..\n");
	}

	return program;
}