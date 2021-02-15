#ifndef H_GPU_MEMORY
#define H_GPU_MEMORY

#include "Buffer.h"
#include <unordered_map>

namespace Bta
{
	namespace Graphic
	{
		class GPUMemory
		{
			private:
				static GPUMemory* s_pGpu;
				GPUMemory() {};
				std::unordered_map<void*, Buffer*> m_oGPUDataBinding;

			public:
				static GPUMemory* GetInstance();
				BasicBuffer* AllocateMemory(BasicBuffer::Desc oDesc, void* pSubject = nullptr);
				Image* AllocateMemory(Image::Desc oDesc, void* pSubject = nullptr);
				Buffer* FetchBuffer(void* pSubject);
		};
	}
}

#endif
