#ifndef H_TEXTURE
#define H_TEXTURE
#include <cstdint>
#include "../Include/vulkan/vulkan_core.h"
#include "Buffer.h"

namespace Bta
{
	namespace Graphic
	{
		class Texture
		{
			public:
				enum EChannel
				{
					R = 0,
					G,
					B,
					A
				};

				Texture(Image::Desc oImgDesc, void* pBuffer);
				~Texture();

				std::shared_ptr<Image> GetImage() { return m_xImage; }

				float GetValue(EChannel eChannel, int iX, int iY);
				void SetValue(EChannel eChannel, int iX, int iY, float fValue);
				void UpdateToGPU();

			private:
				int m_iWidth;
				int m_iHeight;
				unsigned char* m_pBuffer;
				VkFormat m_eFormat;
				std::shared_ptr<Image> m_xImage;
		};
	}
}

#endif
