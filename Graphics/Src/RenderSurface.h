#ifndef H_RENDER_SURFACE
#define H_RENDER_SURFACE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Window
{
	class RenderSurface
	{
	public:
		struct Desc
		{
			int iWidth;
			int iHeight;
			const char* sWindowName;
			VkInstance* pInstance;
			GLFWframebuffersizefun pCallback;
		};
		RenderSurface(Desc& oDesc);

		void GetWindowSize (int& iWidth, int& iHeight);
		GLFWwindow* const GetWindow() const { return m_pWindow; }
		GLFWwindow* GetModifiableWindow() { return m_pWindow; }
		const VkSurfaceKHR* const GetSurface() const { return m_pSurface; }
		static void Init();

	private:
		GLFWwindow* m_pWindow;
		VkSurfaceKHR* m_pSurface;
		static bool s_bGlfwInitialized;
	};
}

#endif H_RENDER_SURFACE

