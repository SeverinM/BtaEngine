#ifndef H_GPU_MEMORY
#define H_GPU_MEMORY

#include "Buffer.h"
#include <list>
#include <utility>
#include <memory>

namespace Bta
{
	namespace Graphic
	{
		class GPUMemory
		{
			private:
				static GPUMemory* s_pGpu;
				GPUMemory() {};
				std::list< std::shared_ptr<Buffer>> m_oGPUDatas;

			public:
				static GPUMemory* GetInstance();
				std::shared_ptr<BasicBuffer> AllocateMemory(BasicBuffer::Desc oDesc);
				std::shared_ptr<Image> AllocateMemory(Image::Desc oDesc);
		};
	}
}

#endif
