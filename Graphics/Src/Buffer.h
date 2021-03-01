#ifndef H_BUFFER
#define H_BUFFER
#include "Globals.h"
#include "GraphicDevice.h"

namespace Bta
{
	namespace Graphic
	{
		class CommandFactory;

		class Buffer
		{
		public:

			enum TypeBuffer
			{
				E_IMAGE,
				E_VERTEX
			};

			static size_t GetMemorySize(VkFormat eFormat);
			const VkDeviceMemory* GetMemory() { return m_pMemory; }
			VkDeviceSize GetAllocatedMemorySize() { return m_iMemorySize; }
			VkDeviceSize GetTrueMemorySize() { return m_iSizeUnit * m_iUnitCount; }
			VkDeviceSize GetSizeUnit() { return m_iSizeUnit; }
			int GetUnitCount() { return m_iUnitCount; }
			void CopyFromMemory(void* pData);
			void CopyFromMemory(void* pData, uint64_t iOffset, uint64_t iSize);
			void CopyFromMemory(void* pSrc, int iWidth, int iPitch, int iHeight)
			{
				char* pSrcChar = (char*)pSrc;
				void* pDst;
				vkMapMemory(*Globals::g_pDevice->GetLogicalDevice(), *m_pMemory, 0, GetTrueMemorySize(), 0, &pDst);
				char* pDstChar = (char*)pDst;

				for (int i = 0; i < iHeight; i++)
				{
					memcpy(pDstChar, pSrcChar, iWidth);
					pDstChar += iPitch;
					pSrcChar += iWidth;
				}

				vkUnmapMemory(*Globals::g_pDevice->GetLogicalDevice(), *m_pMemory);
			}

		protected:
			uint32_t FindMemoryType(uint32_t iTypeFilter, VkMemoryPropertyFlags oProperties);
			VkDeviceMemory* m_pMemory;
			VkDeviceSize m_iSizeUnit;
			uint32_t m_iUnitCount;
			VkDeviceSize m_iMemorySize;
		};

		class BasicBuffer : public Buffer
		{
			friend class GPUMemory;
			public:
				struct Desc
				{
					uint32_t iUnitCount;
					VkBufferUsageFlags eUsage;
					VkMemoryPropertyFlags oPropertyFlags;
					VkDeviceSize iUnitSize;
				};
				virtual ~BasicBuffer();

				const VkBuffer* GetBuffer() { return m_pBuffer; };
				void SendCopyCommand(BasicBuffer* pDst, CommandFactory* pFactory);
				void Reallocate(uint32_t iUnitCount, VkDeviceSize iUnitSize);

			protected:
				BasicBuffer(Desc& oDesc);
				VkBuffer* m_pBuffer;
				VkBufferUsageFlags m_oUsageFlag;
				VkSharingMode m_oSharingMode;
				VkMemoryPropertyFlags m_oProperty;
		};

		class Image : public Buffer
		{
			friend class GPUMemory;
			public:
				struct Desc
				{
					Desc() : eSampleFlag(VK_SAMPLE_COUNT_1_BIT), eFormat(VK_FORMAT_R8G8B8A8_SRGB), eTiling(VK_IMAGE_TILING_OPTIMAL), iLayerCount(1), bEnableMip(false), bIsCubemap(false),
						iWidth(0), iHeight(0), eUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT), eProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), eAspect(VK_IMAGE_ASPECT_COLOR_BIT) {}

					VkSampleCountFlagBits eSampleFlag;
					int iWidth;
					int iHeight;
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
				};

				struct FromFileDesc
				{
					VkSampleCountFlagBits eSampleFlag;
					VkFormat eFormat;
					VkImageTiling eTiling;
					VkImageAspectFlags eAspect;
					bool bEnableMip;
				};

				struct FromBufferDesc
				{
					VkSampleCountFlagBits eSampleFlag;
					VkFormat eFormat;
					VkImageTiling eTiling;
					VkImageAspectFlags eAspect;
					bool bEnableMip;
					void* pBuffer;
					uint16_t iHeight;
					uint16_t iWidth;
					bool bForceSize;
				};

				virtual ~Image();
				static Image* CreateFromFile(std::string sFilename, FromFileDesc& oDesc);
				static Image* CreateCubeMap(std::string sFilenames[6], FromFileDesc& oDesc);
				static Image* CreateFromBuffer(FromBufferDesc& oDesc);

				void TransitionLayout(VkImageLayout eOldLayout, VkImageLayout eNewLayout, int iMipLevel);

				//Getter
				const VkSampler* GetSampler() { return &m_eSampler; }
				const VkImageView* GetImageView() { return &m_oImageView; }
				int GetMipLevel() { return m_iMipLevel; }
				int GetHeight() { return m_iHeight; };
				int GetWidth() { return m_iWidth; };
				const VkImage* GetImage() { return &m_oImage; }

				void SendCopyCommand(BasicBuffer* pBuffer);
				void GenerateMipsInterface(MipDesc& oDesc);

			protected:

				Image(Desc& oDesc);
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
				int m_iRowPitch;
				BasicBuffer* m_pBuffer;
			};
	}
}

#endif