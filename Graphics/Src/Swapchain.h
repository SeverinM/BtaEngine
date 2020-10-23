#ifndef H_SWAP_CHAIN
#define H_SWAP_CHAIN
#include "Resizable.h"
#include "GraphicDevice.h"
#include "GraphicWrapper.h"

class Swapchain : public Resizable
{
public:
	struct Desc
	{
		GraphicWrapper* pGraphicWrapper;
		VkFormat eImagesFormat;
		VkColorSpaceKHR eColorspace;
		VkPresentModeKHR ePresentMode;
		int iImageLayers;
	};

	Swapchain(Desc& oDesc);
	void Create(Desc& oDesc);
	~Swapchain();
	void Free() override;
	virtual void Recreate(int iNewWidth, int iNewHeight, void* pData) override;
	int GetNumberImages() const { return m_oImages.size(); }
	const VkSwapchainKHR* GetSwapchain() const { return &m_oSwapchain; }
	VkFormat GetFormat() const { return m_eFormat; }
	std::vector<VkImageView> GetImageViews()const { return m_oViews; }

protected:
	Desc* m_pRecreate;
	std::vector<VkImage> m_oImages;
	std::vector<VkImageView> m_oViews;
	VkFormat m_eFormat;
	VkSwapchainKHR m_oSwapchain;
	void CreateViews(Desc& oDesc);
};

#endif // ! H_SWAP_CHAIN