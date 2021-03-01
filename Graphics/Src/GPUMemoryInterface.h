#ifndef H_GPU_MEMORY
#define H_GPU_MEMORY

#include "Buffer.h"
#include <list>
#include <utility>

namespace Bta
{
	namespace Graphic
	{
		class GPUMemory
		{
			private:
				static GPUMemory* s_pGpu;
				GPUMemory() {};
				std::list<Buffer*> m_oGPUDatas;

			public:
				static GPUMemory* GetInstance();
				BasicBuffer* AllocateMemory(BasicBuffer::Desc oDesc);
				Image* AllocateMemory(Image::Desc oDesc);
				void FreeMemory(Buffer* pBuffer);
		};
	}
}

#endif
