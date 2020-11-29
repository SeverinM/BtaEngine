#ifndef H_GRAPHIC_UTILS
#define H_GRAPHIC_UTILS
#include "BasicWrapper.h"
#include "GLM/glm.hpp"
#include "RenderBatch.h"
#include "ShaderTags.h"
#include "Globals.h"

namespace Bta
{
	namespace Utils
	{
		class GraphicUtils
		{
		public:

			static RenderBatch* s_pBatch;
			static CommandFactory* s_pFactory;
			static DelayedCommands* s_pDelay;

			static void DisplayDebugSphere(float fRadius, glm::vec4 vColor, float fDuration, BasicWrapper* pWrapper);
			static void DisplayDebugLine(glm::vec3 vStart, glm::vec3 vEnd, glm::vec4 vColor, float fDuration);
		};
	}
}

#endif
