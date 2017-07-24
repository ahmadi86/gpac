#include <gpac/rift_manager_app.h>

static void OVR_CDECL ovr_LogCallback(uintptr_t userData, int level, const char* message)
{
	printf("%s\n", message);

	switch (level)
	{
	case ovrLogLevel_Debug:
	case ovrLogLevel_Info:
	case ovrLogLevel_Error:
	default:
		break;
	}
}

void  gf_ovr_rift_manager_constructor(RiftManagerApp* gf_ovr_RMA)
{
	ovrInitParams params = { 0, 0, NULL, 0, 0, OVR_ON64("") };
	params.LogCallback = ovr_LogCallback;
	ovrResult error = ovr_Initialize(NULL);

	if (!OVR_SUCCESS(error))
	{
		printf("ovr_rift_manager_init, Unable to initialize HMD\n");
		return;
	}

	error = ovr_Create(&gf_ovr_RMA->_session, &gf_ovr_RMA->_luid);
	if (!OVR_SUCCESS(error))
	{
		printf("ovr_rift_manager_init, Unable to create HMD session\n");
		return;
	}

	printf("ovr_rift_manager_init, res=%d\n", error);
	printf("ovr_rift_manager_init, ovrSession address is %p\n", gf_ovr_RMA->_session);

	gf_ovr_RMA->_hmdDesc = ovr_GetHmdDesc(gf_ovr_RMA->_session);

	int eye = 0;
	printf("ovr_rift_manager_constructor, eye=%d, fov_l=%f, tex_r=%f\n", eye, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].LeftTan, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].RightTan);
}

void gf_ovr_rift_manager_deconstructor(RiftManagerApp* gf_ovr_RMA)
{
	ovr_Destroy(gf_ovr_RMA->_session);
	gf_ovr_RMA->_session = NULL;
}