#include "BasicWrapper.h"
#include "Globals.h"
#include "SyncObjects.h"
#include <bitset>
#include <cstdlib>
#include "StringUtils.h"
#include "Parser.h"
#include "FileUtils.h"

int main()
{
	Bta::Graphic::Parser::Desc oParserDesc;
	oParserDesc.sFileName = "./GraphicContext.json";
	Bta::Graphic::Parser* pParser = new Bta::Graphic::Parser(oParserDesc);
	pParser->InitGlobals();
	pParser->RecordTemplates();

	GraphicWrapper::Desc oDesc;
	oDesc.bEnableDebug = true;
	oDesc.oRequiredExtensionsDevice = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	Bta::Graphic::BasicWrapper oWrapper(oDesc);
	oWrapper.Init();

	SyncObjects::Desc oSyncDesc;
	oSyncDesc.iFrameOnFlight = 2;
	oSyncDesc.iNumberImages = (int)oWrapper.GetSwapchain()->GetImageViews().size();
	SyncObjects* pSyncObj = new SyncObjects(oSyncDesc);

	bool bExit = true;
	while (bExit && !glfwWindowShouldClose(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetModifiableWindow()))
	{
		glfwPollEvents();
		bExit = oWrapper.Render(pSyncObj);
	}
	vkDeviceWaitIdle(*Bta::Graphic::Globals::g_pDevice->GetLogicalDevice());

	return 0;
}