#ifndef H_BASIC_WRAPPER
#define H_BASIC_WRAPPER
#include "GraphicWrapper.h"
#include "GraphicDevice.h"
#include "Swapchain.h"
#include "GLM/glm.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "GLM/gtc/matrix_transform.hpp"
#include "Framebuffer.h"
#include "Mesh.h"
#include "Camera.h"
#include "ImGuiWrapper.h"
#include <queue>
#include "DescriptorWrapper.h"
#include "DelayedCommands.h"

class Pipeline;
class CommandFactory;
class RenderPass;
class InputHandling;
class RenderBatch;
class RenderBatchesHandler;

class BasicWrapper : public GraphicWrapper
{
	friend class InputHandling;
	friend class ImGuiWrapper;

public :

	void CreateInstance() override;
	void CreateGraphicDevice() override;
	void CreateSwapChain() override;
	void CreateGraphicPipeline() override;
	void CreateRenderPass() override;
	void CreateCommandBuffer() override;
	void InitBatch();
	void InitFramebuffer();
	void FillDescriptorsBuffer();
	void InitImGui();

	bool Render(SyncObjects* pSync) override;
	static void ResizeWindow(GLFWwindow* pWindow, int iWidth , int iHeight);
	static void RenderGui(BasicWrapper* pWrapper);
	void RecreateSwapChain();

	RenderBatchesHandler* GetHandler() { return m_pHandler; }
	RenderBatchesHandler* GetDebugHandler() { return m_pDebugHandler; }
	DelayedCommands* GetDelayedCommands() { return &m_oCommandsQueue; }
	CommandFactory* GetFactory() { return m_pFactory; }
	DescriptorPool* GetDescriptorPool() { return m_pPool; }
	RenderPass* GetRenderpass() { return m_pRenderpass; }

	BasicWrapper(GraphicWrapper::Desc& oDesc)
	{
		m_pHandling = nullptr;
		m_pHandler = nullptr;
		m_pSwapchain = nullptr;
		m_pRenderpass = nullptr;
		m_pPool = nullptr;
		m_pImGui = nullptr;
		m_pFactory = nullptr;
		m_pDesc = &oDesc;
	}
	virtual ~BasicWrapper();

	const Swapchain* const GetSwapchain() const { return m_pSwapchain; }

protected:
	Swapchain* m_pSwapchain;
	RenderPass* m_pRenderpass;
	CommandFactory* m_pFactory;
	DescriptorPool* m_pPool;
	ImGuiWrapper* m_pImGui;
	InputHandling* m_pHandling;

	RenderPass* m_pDebugRenderpass;

	BasicBuffer* m_pSkyBuffer;
	glm::mat4 m_mModel;

	Mesh::StrongPtr m_xMesh;
	Mesh::StrongPtr m_xMeshSky;

	Mesh::StrongPtr m_xCubeMesh;
	Mesh::StrongPtr m_xCubeMeshChild;

	DelayedCommands m_oCommandsQueue;

	static bool s_bFramebufferResized;
	bool m_bAdd;

	RenderBatchesHandler* m_pHandler;
	RenderBatchesHandler* m_pDebugHandler;

	std::vector< std::shared_ptr<Image> > m_oAllDepths;
	std::vector< std::shared_ptr<Image> > m_oAllMultisample;
	std::vector< Framebuffer* > m_oFramebuffers;
	std::vector< Framebuffer* > m_oFramebuffersDebug;
};

#endif
