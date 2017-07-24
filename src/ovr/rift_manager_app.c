#include <gpac/rift_manager_app.h>

#ifdef _MSC_VER
#define __FUNCTION_NAME__ __FUNCTION__
#else
#define __FUNCTION_NAME__ __func__
#endif


static void OVR_CDECL gf_ovr_LogCallback(uintptr_t userData, int level, const char* message)
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

	// Ahmed: Should make sure struct members are initialized once
	memset(gf_ovr_RMA, 0, sizeof(RiftManagerApp));

	ovrInitParams params = { 0, 0, NULL, 0, 0, OVR_ON64("") };
	params.LogCallback = gf_ovr_LogCallback;
	ovrResult error = ovr_Initialize(NULL);

	if (!OVR_SUCCESS(error))
	{
		printf("%s: Unable to initialize HMD\n", __FUNCTION_NAME__);
		return;
	}

	error = ovr_Create(&gf_ovr_RMA->_session, &gf_ovr_RMA->_luid);
	if (!OVR_SUCCESS(error))
	{
		printf("%s: Unable to create HMD session\n", __FUNCTION_NAME__);
		return;
	}

	printf("%s: res=%d\n", __FUNCTION_NAME__, error);
	printf("%s: ovrSession address is %p\n", __FUNCTION_NAME__, gf_ovr_RMA->_session);

	gf_ovr_RMA->_hmdDesc = ovr_GetHmdDesc(gf_ovr_RMA->_session);

	int eye = 0;
	printf("%s: eye=%d, fov_l=%f, tex_r=%f\n", __FUNCTION_NAME__, eye, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].LeftTan, gf_ovr_RMA->_hmdDesc.DefaultEyeFov[eye].RightTan);
}

void gf_ovr_rift_manager_deconstructor(RiftManagerApp* gf_ovr_RMA)
{
	ovr_Destroy(gf_ovr_RMA->_session);
	gf_ovr_RMA->_session = NULL;
}