#include "RenderSurface.h"
#include <iostream>

namespace Window
{
	bool RenderSurface::s_bGlfwInitialized(false);

	void RenderSurface::Init()
	{
		if (s_bGlfwInitialized)
			return;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		s_bGlfwInitialized = true;
	}

	RenderSurface::RenderSurface(Desc& oDesc)
	{
		if (!s_bGlfwInitialized)
			Init();

		m_pWindow = glfwCreateWindow(oDesc.iWidth, oDesc.iHeight, oDesc.sWindowName, nullptr, nullptr);

		m_pSurface = new VkSurfaceKHR();
		if (glfwCreateWindowSurface(*oDesc.pInstance, m_pWindow, nullptr, m_pSurface))
		{
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void RenderSurface::GetWindowSize(int& iWidth, int& iHeight)
	{
		glfwGetWindowSize(m_pWindow, &iWidth, &iHeight);
	}

}

