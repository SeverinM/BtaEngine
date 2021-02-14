#ifndef H_DATA_BINDING
#define H_DATA_BINDING

#include "Buffer.h"
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		class GPUDataBinding
		{
			private:
				std::shared_ptr<Buffer> m_xBuffer;
				uint64_t m_iOffset;

			public:
				GPUDataBinding(std::shared_ptr<Buffer> xBuffer, uint64_t iOffset = 0) : m_xBuffer(xBuffer), m_iOffset(iOffset){}

				void SetDataGPU(uint64_t iOffset, void* pData, uint64_t iSize)
				{
					m_xBuffer->CopyFromMemory(pData, Bta::Graphic::Globals::g_pDevice, iOffset + m_iOffset, iSize);
				}
		};
	}
}

#endif
