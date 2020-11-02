#include "InputHandling.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"
#include "Globals.h"

InputHandling::InputHandling(Desc& oDesc)
{
	glfwSetKeyCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessKeyboard);
	glfwSetCursorPosCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouse);
	glfwSetMouseButtonCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouseButtons);
	glfwSetWindowUserPointer(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), this);
	m_pWrapper = oDesc.pWrapper;
	m_fPreviousY = m_fPreviousX = -1.0f;
	m_bRightClickEnabled = false;
}

void InputHandling::ProcessKeyboard(GLFWwindow* pwindow, int iKey, int iScancode, int iAction, int iMods)
{
	InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);

	glm::vec3 vTranslate(0.0f);
	glm::vec3 vCamForward = pHandling->m_pWrapper->m_pCamera->GetTransform()->GetForward();
	glm::vec3 vCamRight = pHandling->m_pWrapper->m_pCamera->GetTransform()->GetRight();
	
	if (glfwGetKey(pwindow, GLFW_KEY_W ) == GLFW_PRESS)
	{
		vTranslate += vCamForward * pHandling->m_pWrapper->m_pCamera->GetMoveSpeed() * Graphics::Globals::s_fElapsed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		vTranslate += -vCamForward * pHandling->m_pWrapper->m_pCamera->GetMoveSpeed() * Graphics::Globals::s_fElapsed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		vTranslate += vCamRight * pHandling->m_pWrapper->m_pCamera->GetMoveSpeed() * Graphics::Globals::s_fElapsed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		vTranslate += -vCamRight * pHandling->m_pWrapper->m_pCamera->GetMoveSpeed() * Graphics::Globals::s_fElapsed;
	}

	if (vTranslate.length() != 0)
		pHandling->m_pWrapper->m_pCamera->GetTransform()->SetPosition(vTranslate, true);
}

void InputHandling::ProcessMouse(GLFWwindow* pwindow, double xpos, double ypos)
{
	InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);
	float fDeltaX = (pHandling->m_fPreviousX < 0 ? 0.0f : pHandling->m_fPreviousX - xpos);
	float fDeltaY = (pHandling->m_fPreviousY < 0 ? 0.0f : pHandling->m_fPreviousY - ypos);

	if (pHandling->m_bRightClickEnabled)
	{
		pHandling->m_pWrapper->m_pCamera->GetTransform()->Rotate(glm::vec3(0, 0, 1),fDeltaX * pHandling->m_pWrapper->m_pCamera->GetRotateSpeed() * Graphics::Globals::s_fElapsed);
		pHandling->m_pWrapper->m_pCamera->GetTransform()->Rotate(pHandling->m_pWrapper->m_pCamera->GetTransform()->GetRight(), fDeltaY * pHandling->m_pWrapper->m_pCamera->GetRotateSpeed() * Graphics::Globals::s_fElapsed);
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
