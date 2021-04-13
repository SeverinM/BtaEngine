#ifndef H_SUB_RENDER_BATCH
#define H_SUB_RENDER_BATCH

#include <string>
#include <unordered_map>
#include "GPUMemoryBinding.h"
#include <memory>

namespace Bta
{
	namespace Graphic
	{
		class RenderBatch;
		class MeshComponent;
		class MaterialComponent;
		class MeshComponent;
		class CameraComponent;
		class Pipeline;
		class DescriptorSetWrapper;

		class SubRenderBatch
		{
			friend class RenderBatch;

			public:
				struct Desc
				{
					CameraComponent* pCamera;
					std::vector<std::string> oShaderFiles;
					std::vector<std::string> oCompiledShaderFiles;
					bool bWriteDepth;
					bool bTestDepth;
					VkPrimitiveTopology eVerticesAssemblyMode;
					VkPolygonMode eFillMode;
					VkCullModeFlags eCulling;
					VkCompareOp eDepthTestMethod;
				};

				SubRenderBatch(SubRenderBatch::Desc oDesc);
				MaterialComponent* AddMesh(MeshComponent* pComponent);
				void SetCamera(CameraComponent* pCamera);

			private:
				void FillCommandBuffer(VkCommandBuffer& oBuffer);
				void CreatePipeline(RenderBatch* m_pBatch, int iIndex);

				Pipeline* m_pPipeline;
				CameraComponent* m_pCamera;
				SubRenderBatch::Desc m_oDesc;
				bool m_bEnabled;
				std::unordered_map<MeshComponent*, DescriptorSetWrapper*> m_oAllMeshes;
		};
	}
}

#endif
