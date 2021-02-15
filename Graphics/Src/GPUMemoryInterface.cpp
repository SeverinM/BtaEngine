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

		BasicBuffer* GPUMemory::AllocateMemory(BasicBuffer::Desc oDesc, void* pSubject)
		{
			BasicBuffer* pBuffer = new BasicBuffer(oDesc);
			m_oGPUDataBinding[pSubject] = pBuffer;
			return pBuffer;
		}

		Image* GPUMemory::AllocateMemory(Image::Desc oDesc, void* pSubject)
		{
			Image* pImage = new Image(oDesc);
			m_oGPUDataBinding[pSubject] = pImage;
			return pImage;
		}

		Buffer* GPUMemory::FetchBuffer(void* pSubject)
		{
			if (m_oGPUDataBinding.count(pSubject) == 0)
				return nullptr;

			return m_oGPUDataBinding[pSubject];
		}

	}
}

