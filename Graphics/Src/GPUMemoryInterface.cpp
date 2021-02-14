#include "GPUMemoryInterface.h"

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

		BasicBuffer* GPUMemory::AllocateMemory(BasicBuffer::Desc oDesc)
		{
			BasicBuffer* pBuffer = new BasicBuffer(oDesc);
			m_oAllBuffers.push_back(pBuffer);
			return pBuffer;
		}

		Image* GPUMemory::AllocateMemory(Image::Desc oDesc)
		{
			Image* pImage = new Image(oDesc);
			m_oAllBuffers.push_back(pImage);
			return pImage;
		}

	}
}

