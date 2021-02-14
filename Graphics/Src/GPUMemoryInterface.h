#ifndef H_GPU_MEMORY
#define H_GPU_MEMORY

#include "Buffer.h"

namespace Bta
{
	namespace Graphic
	{
		class GPUMemory
		{
			private:
				static GPUMemory* s_pGpu;
				GPUMemory() {};
				std::vector<Buffer*> m_oAllBuffers;

			public:
				static GPUMemory* GetInstance();
				BasicBuffer* AllocateMemory(BasicBuffer::Desc oDesc);
				Image* AllocateMemory(Image::Desc oDesc);
		};
	}
}

#endif
