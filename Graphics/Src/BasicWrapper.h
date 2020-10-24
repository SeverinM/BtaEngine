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

class Pipeline;
class CommandFactory;
class RenderPass;

class BasicWrapper : public GraphicWrapper
{
public :
	void CreateInstance() override;
	void CreateGraphicDevice() override;
	void CreateSwapChain() override;
	void CreateGraphicPipeline() override;
	void CreateRenderPass() override;
	void CreateCommandBuffer() override;
	void InitVerticesBuffers();
	void InitCommands();
	void InitFramebuffer();
	void FillDescriptorsBuffer();

	bool Render(SyncObjects* pSync) override;
	void ResizeWindow();

	BasicWrapper(GraphicWrapper::Desc& oDesc)
	{
		m_pDesc = &oDesc;
		m_pDevice = nullptr;
	}
	virtual ~BasicWrapper();

	const Swapchain* const GetSwapchain() const { return m_pSwapchain; }
	Pipeline* GetPipeline() const { return m_pPipeline; }
	static BasicWrapper* GetInstance() { return s_pInstance; }

protected:
	static BasicWrapper* s_pInstance;

	Camera* m_pCamera;
	Swapchain* m_pSwapchain;
	RenderPass* m_pRenderpass;
	Pipeline* m_pPipeline;
	CommandFactory* m_pFactory;
	DescriptorPool* m_pPool;
	RenderModel* m_pRenderModel;
	RenderModel* m_pRenderModelSky;
	Pipeline* m_pSkyboxPipeline;
	std::vector< DescriptorPool::UpdateSubDesc> m_oInputDatas;
	std::vector< DescriptorPool::UpdateSubDesc> m_oInputDatasSky;

	std::vector < std::vector < DescriptorPool::BufferDesc> > m_oPrototype;
	std::vector < std::vector < DescriptorPool::BufferDesc> > m_oPrototypeSkybox;
	std::vector< Buffer* > m_oAllVertexBuffers;
	std::vector< Buffer*> m_oAllVertexBuffersSky;
	std::vector<BasicBuffer*> m_oMPMatrices;
	std::vector<BasicBuffer*> m_oAllMatricesSky;
	std::vector<BasicBuffer* >m_oAllMatricesInstance;

	std::vector< VkCommandBuffer > m_oAllDrawCommands;
	std::vector< Image* > m_oAllDepths;
	std::vector<Image*> m_oAllMultisample;
	std::vector< Framebuffer* > m_oFramebuffers;
	std::vector< VkDescriptorSet > m_oDescriptors;
	std::vector<VkDescriptorSet> m_oDescriptorsSky;

	int m_iInstanceCount;

	void UpdateUniformBuffer(int iImageIndex);

	void GetMatrices(glm::mat4& vView, glm::mat4& vProjection, std::vector<glm::mat4>& oModels, int iSize);

	struct MP
	{
		glm::mat4 vModel;
		glm::mat4 vProjection;
	};
	MP GetMatricesSky();

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
