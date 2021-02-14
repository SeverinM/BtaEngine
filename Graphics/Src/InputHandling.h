#ifndef H_INPUT_HANDLING
#define H_INPUT_HANDLING
#include "BasicWrapper.h"
#include <set>

namespace Bta
{
	namespace Graphic
	{
		class InputHandling
		{
		public:
			InputHandling();
			static void ProcessKeyboard(GLFWwindow* pwindow, int iKey, int iScancode, int iAction, int iMods);
			static void ProcessMouse(GLFWwindow* pwindow, double xpos, double ypos);
			static void ProcessMouseButtons(GLFWwindow* pwindow, int button, int action, int mods);

		protected:
			double m_fPreviousX;
			double m_fPreviousY;

			bool m_bRightClickEnabled;
		};
	}
}

#endif

