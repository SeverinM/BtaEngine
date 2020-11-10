#include "BasicWrapper.h"
#include "Globals.h"
#include "SyncObjects.h"
#include <bitset>
#include <cstdlib>
#include "StringUtils.h"
#include "Font.h"

int main()
{
	GraphicWrapper::Desc oDesc;
	oDesc.bEnableDebug = true;
	oDesc.oRequiredExtensionsDevice = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	BasicWrapper oWrapper(oDesc);
	oWrapper.Init();

	SyncObjects::Desc oSyncDesc;
	oSyncDesc.iFrameOnFlight = 2;
	oSyncDesc.iNumberImages = (int)oWrapper.GetSwapchain()->GetImageViews().size();
	oSyncDesc.pWrapper = &oWrapper;
	SyncObjects* pSyncObj = new SyncObjects(oSyncDesc);

	bool bExit = true;
	while (bExit && !glfwWindowShouldClose(oWrapper.GetModifiableDevice()->GetModifiableRenderSurface()->GetModifiableWindow()))
	{
		glfwPollEvents();
		bExit = oWrapper.Render(pSyncObj);
	}
	vkDeviceWaitIdle(*oWrapper.GetDevice()->GetLogicalDevice());

	return 0;
}