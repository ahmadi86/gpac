#ifndef OVR_RIFT_MANAGER_APP
#define OVR_RIFT_MANAGER_APP

#include <stdio.h>

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

typedef struct RiftManagerApp_
{
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

} RiftManagerApp;


static void OVR_CDECL ovr_LogCallback(uintptr_t userData, int level, const char* message);

void gf_ovr_rift_manager_constructor(RiftManagerApp* gf_ovr_RMA);
void gf_ovr_rift_manager_deconstructor(RiftManagerApp* gf_ovr_RMA);

#endif
