#ifndef H_BASIC_WRAPPER
#define H_BASIC_WRAPPER
#include "GraphicWrapper.h"
#include "GraphicDevice.h"
#include "Swapchain.h"
#include "GLM/glm.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "GLM/gtc/matrix_transform.hpp"
#include "DescriptorPool.h"
#include "Framebuffer.h"
#include "Model.h"
#include "Camera.h"
#include "ImGuiWrapper.h"

class Pipeline;
class CommandFactory;
class RenderPass;

class BasicWrapper : public GraphicWrapper
{
	friend class ImGuiWrapper;

public :
	void CreateInstance() override;
	void CreateGraphicDevice() override;
	void CreateSwapChain() override;
	void CreateGraphicPipeline() override;
	void CreateRenderPass() override;
	void CreateCommandBuffer() override;
	void InitVerticesBuffers();
	void InitFramebuffer();
	void FillDescriptorsBuffer();
	void InitImGui();

	bool Render(SyncObjects* pSync) override;
	static void ResizeWindow(GLFWwindow* pWindow, int iWidth , int iHeight);
	void RecreateSwapChain();

	BasicWrapper(GraphicWrapper::Desc& oDesc)
	{
		m_pDesc = &oDesc;
		m_pDevice = nullptr;
	}
	virtual ~BasicWrapper();

	const Swapchain* const GetSwapchain() const { return m_pSwapchain; }
	Pipeline* GetPipeline() const { return m_pPipeline; }

protected:

	Camera* m_pCamera;
	Swapchain* m_pSwapchain;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	CommandFactory* m_pFactory;
	DescriptorPool* m_pPool;
	RenderModel* m_pRenderModel;
	RenderModel* m_pRenderModelSky;
	Pipeline* m_pSkyboxPipeline;
	ImGuiWrapper* m_pImGui;
	static bool s_bFramebufferResized;
	std::vector< DescriptorPool::UpdateSubDesc> m_oInputDatas;
	std::vector< DescriptorPool::UpdateSubDesc> m_oInputDatasSky;

	std::vector < std::vector < DescriptorPool::BufferDesc> > m_oPrototype;
	std::vector < std::vector < DescriptorPool::BufferDesc> > m_oPrototypeSkybox;

	std::vector< VkCommandBuffer > m_oAllDrawCommands;
	std::vector< std::shared_ptr<Image> > m_oAllDepths;
	std::vector< std::shared_ptr<Image> > m_oAllMultisample;
	std::vector< Framebuffer* > m_oFramebuffers;

	void UpdateUniformBuffer(int iImageIndex);

	struct Vertex
	{
		glm::vec3 vPos;
		glm::vec2 vTex;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription oBindingDescription{};
			oBindingDescription.binding = 0;
			oBindingDescription.stride = sizeof(Vertex);
			oBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return oBindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> oAttributeDescriptions{};
			oAttributeDescriptions.resize(2);

			oAttributeDescriptions[0].binding = 0;
			oAttributeDescriptions[0].location = 0;
			oAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			oAttributeDescriptions[0].offset = offsetof(Vertex, vPos);

			oAttributeDescriptions[1].binding = 0;
			oAttributeDescriptions[1].location = 1;
			oAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			oAttributeDescriptions[1].offset = offsetof(Vertex, vTex);

			return oAttributeDescriptions;
		}
	};
};

#endif
