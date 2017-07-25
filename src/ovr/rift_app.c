#include <gpac/rift_app.h>"

#ifdef _MSC_VER
#define __FUNCTION_NAME__ __FUNCTION__
#else
#define __FUNCTION_NAME__ __func__
#endif

void gf_ovr_rift_constructor(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	printf("%s\n", __FUNCTION_NAME__);

	// Ahmed: Should make sure struct memebers are initialized once
	memset(gf_ovr_RGA, 0, sizeof(RiftGLApp));

	gf_ovr_RGA->_eyeTexture = NULL;

	printf("%s: Call ovr_rift_manager_constructor\n", __FUNCTION_NAME__);
	gf_ovr_rift_manager_constructor(gf_ovr_RMA);

	printf("%s: Call ovr_glfw_constructor\n", __FUNCTION_NAME__);
	ovr_glfw_constructor(gf_ovr_GLA);  // initialize GLFW

	printf("%s: Call ovr_rift_sc_constructor\n", __FUNCTION_NAME__);
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
		printf("%s: eye=%d, fov_l=%f, tex_r=%f\n", __FUNCTION_NAME__, eye, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].LeftTan, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].RightTan);

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

		printf("%s: eye=%d, tex_w=%d, tex_h=%d\n", __FUNCTION_NAME__, eye, eyeSize.w, eyeSize.h);

		ovrVector2i p;
		p.x = gf_ovr_RGA->_renderTargetSize.w;
		p.y = 0;

		gf_ovr_RGA->_sceneLayer.Viewport[eye].Pos = p;

		printf("%s: Render target size = %d x %d\n", __FUNCTION_NAME__, gf_ovr_RGA->_renderTargetSize.w, gf_ovr_RGA->_renderTargetSize.h);

		gf_ovr_RGA->_renderTargetSize.h = gf_ovr_RGA->_renderTargetSize.h >= eyeSize.h ? gf_ovr_RGA->_renderTargetSize.h : eyeSize.h;
		gf_ovr_RGA->_renderTargetSize.w += eyeSize.w;
	}


	// Make the on screen window 1/4 the resolution of the render target
	gf_ovr_RGA->_mirrorSize = gf_ovr_RGA->_renderTargetSize;
	gf_ovr_RGA->_mirrorSize.h /= 4;
	gf_ovr_RGA->_mirrorSize.w /= 4;
	printf("%s: Mirror target size = %d x %d\n", __FUNCTION_NAME__, gf_ovr_RGA->_mirrorSize.w, gf_ovr_RGA->_mirrorSize.h);
}

int gf_ovr_rift_init_gl(RiftGLApp *gf_ovr_RGA, RiftManagerApp *gf_ovr_RMA)
{
	printf("%s: tex_w=%d, tex_h=%d\n", __FUNCTION_NAME__, gf_ovr_RGA->_renderTargetSize.w, gf_ovr_RGA->_renderTargetSize.h);

	ovr_glfw_init_gl();  // currently does nothing

	// disable the v-sync for buffer swap
	glfwSwapInterval(0);

	//printf("gf_ovr_RGA_init, v-sync for buffer swap disabled.\n");

	//printf("%s: ovrSession address is %p\n", __FUNCTION_NAME__, (void*)gf_ovr_RMA->_session);

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
		printf("%s: Failed to create swap textures (result = %d)\n", __FUNCTION_NAME__, result);
		return -1;
	}

	int length = 0;
	result = ovr_GetTextureSwapChainLength(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, &length);
	if (!OVR_SUCCESS(result) || !length) {
		printf("%s: Unable to count swap chain textures\n", __FUNCTION_NAME__);
		return -1;
	}

	printf("%s: ovr_GetTextureSwapChainLength, length=%d\n", __FUNCTION_NAME__, length);

	for (int i = 0; i < length; ++i) {
		GLuint chainTexId;
		ovr_GetTextureSwapChainBufferGL(gf_ovr_RMA->_session, gf_ovr_RGA->_eyeTexture, i, &chainTexId);

		printf("%s: ovr_GetTextureSwapChainBufferGL, tex_id=%d\n", __FUNCTION_NAME__, chainTexId);

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

	printf("%s: Call ovr_CreateMirrorTextureGL\n", __FUNCTION_NAME__);
	if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(gf_ovr_RMA->_session, &mirrorDesc, &gf_ovr_RGA->_mirrorTexture))) {
		printf("%s: Could not create mirror texture\n", __FUNCTION_NAME__);
	}
	glGenFramebuffers(1, &gf_ovr_RGA->_mirrorFbo);

	gf_ovr_rift_sc_init_gl(gf_ovr_RGA, gf_ovr_RMA);

	// video texture	
	// Y
	gf_ovr_RGA->video_texture_Y = 0;
	glGenTextures(1, &gf_ovr_RGA->video_texture_Y);
	glBindTexture(GL_TEXTURE_2D, gf_ovr_RGA->video_texture_Y);
	//	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	// U
	gf_ovr_RGA->video_texture_U = 0;
	glGenTextures(1, &gf_ovr_RGA->video_texture_U);
	glBindTexture(GL_TEXTURE_2D, gf_ovr_RGA->video_texture_U);
	//	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	// V
	gf_ovr_RGA->video_texture_V = 0;
	glGenTextures(1, &gf_ovr_RGA->video_texture_V);
	glBindTexture(GL_TEXTURE_2D, gf_ovr_RGA->video_texture_V);
	//	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return 0;
}

int gf_ovr_rift_run1(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	printf("%s: Call ovr_rift_pre_create\n", __FUNCTION_NAME__);
	ovr_glfw_pre_create();  // setup OpenGL context version and window hints

	printf("%s: Call ovr_rift_create_rendering_target\n", __FUNCTION_NAME__);
	ovrVector2i sz;
	sz.x = gf_ovr_RGA->_mirrorSize.w;
	sz.y = gf_ovr_RGA->_mirrorSize.h;

	printf("%s: Mirror window size = %d x %d\n", __FUNCTION_NAME__, sz.x, sz.y);

	// actually create the window
	gf_ovr_GLA->window = gf_ovr_rift_create_rendering_target(sz, gf_ovr_GLA->windowPosition);

	if (!gf_ovr_GLA->window) {
		printf("%s: Unable to create OpenGL window\n", __FUNCTION_NAME__);
		return -1;
	}
	else
	{
		printf("%s: We have a window\n", __FUNCTION_NAME__);
	}

	printf("%s: Call ovr_rift_post_create\n", __FUNCTION_NAME__);
	ovr_glfw_post_create(gf_ovr_GLA);  // init GLEW and make context current

	printf("%s: Call ovr_rift_init_gl\n", __FUNCTION_NAME__);
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
		fprintf(stderr, "%s: window is NULL\n", __FUNCTION_NAME__);
	}
	return 0;
}


int gf_ovr_rift_run3(RiftGLApp *gf_ovr_RGA, GLFWApp *gf_ovr_GLA, RiftManagerApp *gf_ovr_RMA)
{
	printf("%s\n", __FUNCTION_NAME__);

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

ovrMatrix4f gf_ovr_get_model_view(ovrPosef pose)
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

		gf_ovr_rift_sc_render(gf_ovr_RGA, gf_ovr_RGA->_eyeProjections[eye], gf_ovr_get_model_view(eyePoses[eye]));
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
}

void gf_ovr_rift_sc_init_gl(RiftGLApp* gf_ovr_RGA, RiftManagerApp* gf_ovr_RMA) {

	fprintf(stderr, "%s\n", __FUNCTION_NAME__);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	ovr_RecenterTrackingOrigin(gf_ovr_RMA->_session);

	// generate a sphere mesh using 30 rings and 30 sectors
	gf_ovr_sphere_init(&gf_ovr_RGA->sphere, 100, 100);
}

void gf_ovr_rift_sc_render(RiftGLApp *gf_ovr_RGA, const ovrMatrix4f projection, const ovrMatrix4f modelView) {

	gf_ovr_RGA->sphere.projection = ovrMatrix4f_Transpose(&projection);
	ovrMatrix4f tmp = ovrMatrix4f_Inverse(&modelView);
	gf_ovr_RGA->sphere.model_view = ovrMatrix4f_Transpose(&tmp);

	gf_ovr_sphere_draw(&gf_ovr_RGA->sphere);

}