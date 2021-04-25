#include "Texture.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "GPUMemoryInterface.h"
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		Texture::Texture(Image::Desc oImgDesc, void* pBuffer)
		{
			m_iWidth = oImgDesc.iWidth;
			m_iHeight = oImgDesc.iHeight;
			m_eFormat = oImgDesc.eFormat;

			if (pBuffer != nullptr)
			{
				m_pBuffer = (unsigned char*)pBuffer;
			}
			else
			{
				m_pBuffer = (unsigned char*)malloc(m_iHeight * m_iWidth * 4 * 4);
			}

			m_xImage = GPUMemory::GetInstance()->AllocateMemory(oImgDesc);
		}

		Texture::~Texture()
		{
			free(m_pBuffer);
		}

		float Texture::GetValue(EChannel eChannel, int iX, int iY)
		{
			float fOutput;
			int iOffset = ( ( iY * m_iWidth ) + iX ) * 4;
			iOffset += eChannel;
			unsigned char* pValue = m_pBuffer + iOffset;
			memcpy(&fOutput, pValue, sizeof(float));
			return fOutput;
		}

		void Texture::SetValue(EChannel eChannel, int iX, int iY, float fValue)
		{
			int iOffset = ((iY * m_iWidth) + iX) * 4;
			iOffset += eChannel;
			unsigned char* pValue = m_pBuffer + iOffset;
			memcpy(pValue, &fValue, sizeof(float));
		}

		void Texture::UpdateToGPU()
		{
			m_xImage->CopyFromMemory(m_pBuffer, 0, m_iWidth * m_iHeight * 4 * 4);
		}
	}
}

