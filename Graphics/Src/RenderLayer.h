#ifndef H_RENDER_LAYER
#define H_RENDER_LAYER
#include "SyncObjects.h"
#include "Buffer.h"
#include "Framebuffer.h"

class BaseLayer
{
public:
	virtual void Render(SyncObjects* pSync) = 0;
};

class SkyBoxRenderLayer : public BaseLayer
{
public:
	struct Desc
	{
		std::vector<Image*> pSwapchainImages;
		std::vector<std::string> sFilenames;
		std::vector<std::string> sShaders;
		GraphicDevice* pDevice;
	};
	SkyBoxRenderLayer(Desc& oDesc);
	virtual void Render(SyncObjects* pSync) override;

protected :
	Image* m_pImage;
};

#endif