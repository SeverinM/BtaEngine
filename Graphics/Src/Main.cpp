#include "Parser.h"
#include "../../Core/Src/TransformComponent.h"
#include "Buffer.h"
#include "Globals.h"
#include "RenderBatch.h"
#include "MeshComponent.h"
#include "Output.h"
#include <vector>
#include "CameraComponent.h"
#include "Texture.h"
#include "CommandFactory.h"
#include "MaterialComponent.h"

int main()
{
	Bta::Graphic::Parser::Desc oParserDesc;
	oParserDesc.sFileName = "./GraphicContext.json";
	Bta::Graphic::Parser* pParser = new Bta::Graphic::Parser(oParserDesc);
	pParser->InitGlobals();

	Bta::Core::Entity* pEntity = new Bta::Core::Entity(nullptr);
	Bta::Graphic::MeshComponent oComponent(pEntity);
	oComponent.AllocateGPUMemory(50,0);
	pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(0, 1, 0), false);

	Bta::Graphic::Vertice oVert;
	oVert.vColor = glm::vec4(255.0f, 255.0f, 255.0f, 1);
	oVert.vNormal = glm::vec3(0, 1, 0);
	oVert.vUV = glm::vec2(0, 0);
	oVert.vPosition = glm::vec3(0.2f, 0, -1);
	oComponent.AddVertice(oVert, 0);
	oVert.vColor = glm::vec4(2.0f, 205.0f, 155.0f, 1);
	oVert.vPosition = glm::vec3(-1, -1, 2);
	oComponent.AddVertice(oVert, 1);
	oVert.vColor = glm::vec4(2.0f, 5.0f, 125.0f, 1);
	oVert.vPosition = glm::vec3(-1, 1, -2);
	oComponent.AddVertice(oVert, 2);

	Bta::Graphic::CameraComponent::Desc oCamDesc;
	oCamDesc.bEnablePerspective = true;
	oCamDesc.fFarPlane = 10;
	oCamDesc.fNearPlane = 0.001f;
	oCamDesc.fFOVDegrees = 25;
	oCamDesc.fRatio = 1;

	Bta::Core::Entity* pEntityCam = new Bta::Core::Entity(nullptr);
	Bta::Graphic::CameraComponent oCamComponent(oCamDesc, pEntityCam);

	Bta::Graphic::RenderBatch::Desc oDesc;
	oDesc.iSampleCount = VK_SAMPLE_COUNT_1_BIT;
	oDesc.bPresentable = true;
	oDesc.oFramebufferLayout = { Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetFormat(), VK_FORMAT_D32_SFLOAT };
	
	Bta::Graphic::SubRenderBatch::Desc oSubDesc;
	oSubDesc.bTestDepth = true;
	oSubDesc.bWriteDepth = true;
	oSubDesc.eCulling = VK_CULL_MODE_NONE;
	oSubDesc.eDepthTestMethod = VK_COMPARE_OP_LESS_OR_EQUAL;
	oSubDesc.eFillMode = VK_POLYGON_MODE_FILL;
	oSubDesc.eVerticesAssemblyMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	oSubDesc.oCompiledShaderFiles = { "./Shader/vert.spv", "./Shader/frag.spv" };
	oSubDesc.oShaderFiles = { "./Shader/Src/vs.vert", "./Shader/Src/fs.frag" };
	oSubDesc.pCamera = &oCamComponent;
	
	oDesc.oSubBatches.push_back(oSubDesc);

	Bta::Graphic::RenderBatch oRenderBatch(oDesc);
	Bta::Graphic::MaterialComponent* pMat = oRenderBatch.GetSubBatches()[0]->AddMesh(&oComponent);
	pMat->CommitChange();

	Bta::Graphic::Globals::g_pOutput->GenerateFramebuffers({ Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetFormat(),VK_FORMAT_D32_SFLOAT }, &oRenderBatch);

	while (!glfwWindowShouldClose(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow()))
	{
		glfwPollEvents();
		Bta::Graphic::Globals::g_pOutput->RenderOneFrame({ &oRenderBatch });
		Bta::Graphic::Globals::g_pOutput->Present();
		Bta::Graphic::Globals::g_pOutput->NextFrame();
	}

	return 0;
}