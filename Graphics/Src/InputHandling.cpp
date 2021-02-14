#include "InputHandling.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include "Globals.h"
#include "RenderBatch.h"
#include "GraphicUtils.h"

namespace Bta
{
	namespace Graphic
	{
		InputHandling::InputHandling()
		{
			glfwSetKeyCallback(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessKeyboard);
			glfwSetCursorPosCallback(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouse);
			glfwSetMouseButtonCallback(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouseButtons);
			glfwSetWindowUserPointer(Bta::Graphic::Globals::g_pDevice->GetModifiableRenderSurface()->GetWindow(), this);
			m_fPreviousY = m_fPreviousX = -1.0f;
			m_bRightClickEnabled = false;
		}

		void InputHandling::ProcessKeyboard(GLFWwindow* pwindow, int iKey, int iScancode, int iAction, int iMods)
		{
			InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);

			glm::vec3 vTranslate(0.0f);
			glm::vec3 vCamForward = Bta::Graphic::Globals::g_pCamera->GetTransform()->GetForward();
			glm::vec3 vCamRight = Bta::Graphic::Globals::g_pCamera->GetTransform()->GetRight();

			if (glfwGetKey(pwindow, GLFW_KEY_W) == GLFW_PRESS)
			{
				vTranslate += vCamForward * Bta::Graphic::Globals::g_pCamera->GetMoveSpeed() * Bta::Graphic::Globals::g_fElapsed;
			}

			if (glfwGetKey(pwindow, GLFW_KEY_S) == GLFW_PRESS)
			{
				vTranslate += -vCamForward * Bta::Graphic::Globals::g_pCamera->GetMoveSpeed() * Bta::Graphic::Globals::g_fElapsed;
			}

			if (glfwGetKey(pwindow, GLFW_KEY_A) == GLFW_PRESS)
			{
				vTranslate += vCamRight * Bta::Graphic::Globals::g_pCamera->GetMoveSpeed() * Bta::Graphic::Globals::g_fElapsed;
			}

			if (glfwGetKey(pwindow, GLFW_KEY_D) == GLFW_PRESS)
			{
				vTranslate += -vCamRight * Bta::Graphic::Globals::g_pCamera->GetMoveSpeed() * Bta::Graphic::Globals::g_fElapsed;
			}

			if (glfwGetKey(pwindow, GLFW_KEY_U) == GLFW_PRESS)
			{
				Bta::Utils::GraphicUtils::DisplayDebugLine(glm::vec3(0), glm::vec3(0, 0, 10), glm::vec4(1, 1, 1, 1), 10.0f);
			}

			if (vTranslate.length() != 0)
				Bta::Graphic::Globals::g_pCamera->GetTransform()->SetPosition(vTranslate, true);
		}

		void InputHandling::ProcessMouse(GLFWwindow* pwindow, double xpos, double ypos)
		{
			InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);
			float fDeltaX = (pHandling->m_fPreviousX < 0 ? 0.0f : pHandling->m_fPreviousX - xpos);
			float fDeltaY = (pHandling->m_fPreviousY < 0 ? 0.0f : pHandling->m_fPreviousY - ypos);

			if (pHandling->m_bRightClickEnabled)
			{
				Bta::Graphic::Globals::g_pCamera->GetTransform()->Rotate(glm::vec3(0, 0, 1), fDeltaX * Bta::Graphic::Globals::g_pCamera->GetRotateSpeed() * Bta::Graphic::Globals::g_fElapsed);
				Bta::Graphic::Globals::g_pCamera->GetTransform()->Rotate(Bta::Graphic::Globals::g_pCamera->GetTransform()->GetRight(), fDeltaY * Bta::Graphic::Globals::g_pCamera->GetRotateSpeed() * Bta::Graphic::Globals::g_fElapsed);
			}

			pHandling->m_fPreviousX = xpos;
			pHandling->m_fPreviousY = ypos;
		}

		void InputHandling::ProcessMouseButtons(GLFWwindow* pwindow, int button, int action, int mods)
		{
			InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);
			int iState = glfwGetMouseButton(pwindow, GLFW_MOUSE_BUTTON_RIGHT);
			pHandling->m_bRightClickEnabled = (iState == GLFW_PRESS);
		}
	}
}
