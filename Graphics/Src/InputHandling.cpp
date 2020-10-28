#include "InputHandling.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

InputHandling::InputHandling(Desc& oDesc)
{
	glfwSetKeyCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessKeyboard);
	glfwSetCursorPosCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouse);
	glfwSetMouseButtonCallback(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), InputHandling::ProcessMouseButtons);
	glfwSetWindowUserPointer(oDesc.pWrapper->m_pDevice->GetModifiableRenderSurface()->GetWindow(), this);
	m_fSpeed = 0.01f;
	m_fRotateSpeed = 0.1f;
	m_pWrapper = oDesc.pWrapper;
	m_fPreviousY = m_fPreviousX = -1.0f;
	m_bRightClickEnabled = false;
}

void InputHandling::ProcessKeyboard(GLFWwindow* pwindow, int iKey, int iScancode, int iAction, int iMods)
{
	InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);

	glm::vec3 vTranslate(0.0f);
	glm::vec3 vCamForward = pHandling->m_pWrapper->m_pCamera->GetForward();
	glm::vec3 vCamRight = pHandling->m_pWrapper->m_pCamera->GetRight();
	
	if (glfwGetKey(pwindow, GLFW_KEY_W ) == GLFW_PRESS)
	{
		vTranslate += vCamForward * pHandling->m_fSpeed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		vTranslate += -vCamForward * pHandling->m_fSpeed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		vTranslate += vCamRight * pHandling->m_fSpeed;
	}

	if (glfwGetKey(pwindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		vTranslate += -vCamRight * pHandling->m_fSpeed;
	}

	pHandling->m_pWrapper->m_pCamera->Translate(vTranslate);
}

void InputHandling::ProcessMouse(GLFWwindow* pwindow, double xpos, double ypos)
{
	InputHandling* pHandling = (InputHandling*)glfwGetWindowUserPointer(pwindow);
	float fDeltaX = (pHandling->m_fPreviousX < 0 ? 0.0f : pHandling->m_fPreviousX - xpos);
	float fDeltaY = (pHandling->m_fPreviousY < 0 ? 0.0f : pHandling->m_fPreviousY - ypos);

	if (pHandling->m_bRightClickEnabled)
	{
		pHandling->m_pWrapper->m_pCamera->Rotate(glm::vec3(0, 0, 1), 10.0f * fDeltaX * pHandling->m_fRotateSpeed);
		pHandling->m_pWrapper->m_pCamera->Rotate(pHandling->m_pWrapper->m_pCamera->GetRight(), 10.0f * fDeltaY * pHandling->m_fRotateSpeed);
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
