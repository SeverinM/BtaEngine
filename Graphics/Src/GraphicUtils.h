#ifndef H_GRAPHIC_UTILS
#define H_GRAPHIC_UTILS
#include "BasicWrapper.h"
#include "GLM/glm.hpp"

#define DEBUG_TAG "Debug"

namespace Bta
{
	namespace Utils
	{
		class GraphicUtils
		{
			static void DisplayDebugSphere(float fRadius, glm::vec4 vColor, float fDuration, BasicWrapper* pWrapper)
			{
				RenderBatch* pBatch = pWrapper->m_pHandler->FindRenderBatch(DEBUG_TAG);

				if (pBatch != nullptr)
				{

				}
			}
		};
	}
}

#endif
