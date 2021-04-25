#include "GPUMemoryInterface.h"
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		GPUMemory* GPUMemory::s_pGpu(nullptr);

		GPUMemory* GPUMemory::GetInstance()
		{
			if (s_pGpu == nullptr)
				s_pGpu = new GPUMemory();

			return s_pGpu;
		}

		std::shared_ptr<BasicBuffer> GPUMemory::AllocateMemory(BasicBuffer::Desc oDesc)
		{
			std::shared_ptr<BasicBuffer> xBuffer = std::shared_ptr<BasicBuffer>( new BasicBuffer(oDesc));
			m_oGPUDatas.push_back(xBuffer);
			return xBuffer;
		}

		std::shared_ptr<Image> GPUMemory::AllocateMemory(Image::Desc oDesc)
		{
			std::shared_ptr<Image> xImage = std::shared_ptr<Image>( new Image(oDesc) );
			m_oGPUDatas.push_back(xImage);
			return xImage;
		}
	}
}

