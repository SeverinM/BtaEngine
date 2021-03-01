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

		BasicBuffer* GPUMemory::AllocateMemory(BasicBuffer::Desc oDesc)
		{
			BasicBuffer* pBuffer = new BasicBuffer(oDesc);
			m_oGPUDatas.push_back(pBuffer);
			return pBuffer;
		}

		Image* GPUMemory::AllocateMemory(Image::Desc oDesc)
		{
			Image* pImage = new Image(oDesc);
			m_oGPUDatas.push_back(pImage);
			return pImage;
		}

		void GPUMemory::FreeMemory(Buffer* pBuffer)
		{
			std::list<Buffer*>::iterator it = std::find(m_oGPUDatas.begin(), m_oGPUDatas.end(),pBuffer);

			if (it != m_oGPUDatas.end())
			{
				Buffer* pBuffer = (*it);
				m_oGPUDatas.erase(it);
				delete pBuffer;
			}		
		}
	}
}

