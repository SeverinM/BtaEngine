#ifndef H_BUFFER
#define H_BUFFER
#include "GraphicWrapper.h"
#include "Resizable.h"

class CommandFactory;

class Buffer : public Resizable
{
public:

	enum TypeBuffer
	{
		E_IMAGE,
		E_VERTEX
	};

	const VkDeviceMemory* GetMemory() { return &m_oMemory; }
	VkDeviceSize GetMemorySize() { return m_iSizeUnit * (VkDeviceSize)m_iUnitCount; }
	VkDeviceSize GetSizeUnit() { return m_iSizeUnit; }
	int GetUnitCount() { return m_iUnitCount; }
	virtual TypeBuffer GetType() = 0;
	void CopyFromMemory(void* pData, GraphicDevice* pDevice);

protected:
	uint32_t FindMemoryType(GraphicWrapper* pWrapper, uint32_t iTypeFilter, VkMemoryPropertyFlags oProperties);
	VkDeviceMemory m_oMemory;
	VkDeviceSize m_iSizeUnit;
	uint32_t m_iUnitCount;

	virtual void Free() override;
	virtual void Recreate(int iNewWidth, int iNewHeight, void* pData) override;
};

class BasicBuffer : public Buffer
{

public:
	struct Desc
	{
		GraphicWrapper* pWrapper;
		uint32_t iUnitCount;
		VkBufferUsageFlags eUsage;
		VkMemoryPropertyFlags oPropertyFlags;
		VkDeviceSize iUnitSize;
	};
	virtual ~BasicBuffer();

	const VkBuffer* GetBuffer() { return &m_oBuffer; };
	BasicBuffer(Desc& oDesc);
	TypeBuffer GetType() { return E_VERTEX; }
	void SendCopyCommand(BasicBuffer* pDst, CommandFactory* pFactory);
	GraphicDevice* m_pDevice;

protected:
	VkBuffer m_oBuffer;
};

class Image : public Buffer
{
public:
	struct Desc
	{
		Desc() : eSampleFlag(VK_SAMPLE_COUNT_1_BIT), eFormat(VK_FORMAT_R8G8B8A8_SRGB), eTiling(VK_IMAGE_TILING_OPTIMAL), iLayerCount(1), bEnableMip(false), bIsCubemap(false) {}

		VkSampleCountFlagBits eSampleFlag;
		GraphicWrapper* pWrapper = nullptr;
		CommandFactory* pFactory = nullptr;
		uint32_t iWidth;
		uint32_t iHeight;
		VkFormat eFormat;
		VkImageTiling eTiling;
		VkImageUsageFlags eUsage;
		VkMemoryPropertyFlags eProperties;
		VkImageAspectFlags eAspect;
		int iLayerCount;
		bool bEnableMip;
		bool bIsCubemap;
	};

	struct MipDesc
	{
		VkFormat eFormat;
		CommandFactory* pFactory;
		GraphicWrapper* pWrapper;
	};

	struct FromFileDesc
	{
		VkSampleCountFlagBits eSampleFlag;
		GraphicWrapper* pWrapper;
		VkFormat eFormat;
		VkImageTiling eTiling;
		VkImageAspectFlags eAspect;
		CommandFactory* pFactory;
		bool bEnableMip;
	};

	Image(Desc& oDesc);
	virtual ~Image();
	static Image* CreateFromFile(std::string sFilename, FromFileDesc& oDesc);
	static Image* CreateCubeMap(std::string sFilenames[6], FromFileDesc& oDesc);

	void TransitionLayout(VkImageLayout eOldLayout, VkImageLayout eNewLayout,CommandFactory* pFactory, int iMipLevel);

	//Getter
	const VkSampler* GetSampler() { return &m_eSampler; }
	const VkImageView* GetImageView() { return &m_oImageView; }
	int GetMipLevel() { return m_iMipLevel; }
	int GetHeight() { return m_iHeight; };
	int GetWidth() { return m_iWidth; };
	TypeBuffer GetType() { return E_IMAGE; }
	const VkImage* GetImage() { return &m_oImage; }


	void SendCopyCommand(BasicBuffer* pBuffer, CommandFactory* pFactory);
	void GenerateMipsInterface(MipDesc& oDesc);

protected :
	void GenerateMips(MipDesc& oDesc);
	void CreateSampler(Desc& oDesc);
	void CreateView(Desc& oDesc, VkImageAspectFlags oAspect);

	VkImageView m_oImageView;
	VkSampler m_eSampler;
	VkImage m_oImage;
	int m_iMipLevel;
	int m_iHeight;
	int m_iWidth;
	bool m_bIsCubemap;
	GraphicDevice* m_pDevice;
	BasicBuffer* m_pBuffer;
};

#endif