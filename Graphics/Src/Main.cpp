#include "Parser.h"
#include "./TransformComponentGPU.h"
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
#include "GraphicUtils.h"
#include "ImGuiWrapper.h"

int main()
{
	Bta::Graphic::Parser::Desc oParserDesc;
	oParserDesc.sFileName = "./GraphicContext.json";
	Bta::Graphic::Parser* pParser = new Bta::Graphic::Parser(oParserDesc);
	pParser->InitGlobals();

	Bta::Core::Entity* pEntity = new Bta::Core::Entity(nullptr);
	Bta::Graphic::MeshComponent oMeshComponent(pEntity);
	Bta::Graphic::TransformComponentGPU oTransformGPU;
	pEntity->AddExistingComponent(&oMeshComponent);
	pEntity->AddExistingComponent(&oTransformGPU);
	pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(0.5f, -0.5f, 2), false);
	pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();

	Bta::Graphic::GraphicUtils::OutputMesh oBox = Bta::Graphic::GraphicUtils::CreateBox();
	oMeshComponent.AllocateGPUMemory(oBox.vertices.size(), oBox.indices.size());

	for (int i = 0; i < oBox.vertices.size(); i++)
	{
		oBox.vertices[i].vColor = glm::vec4( (rand() % 255) / (float)255, (rand() % 255) / (float)255, (rand() % 255) / (float)255 , 1.0f);
		oMeshComponent.SetVertice(oBox.vertices[i], i);
	}

	for (int i = 0; i < oBox.indices.size(); i++)
	{
		oMeshComponent.SetIndex(oBox.indices[i], i);
	}

	Bta::Graphic::CameraComponent::Desc oCamDesc;
	oCamDesc.bEnablePerspective = true;
	oCamDesc.fFarPlane = 10;
	oCamDesc.fNearPlane = 0.001f;
	oCamDesc.fFOVDegrees = 25;
	oCamDesc.fRatio = 1;

	Bta::Core::Entity* pEntityCam = new Bta::Core::Entity(nullptr);
	Bta::Graphic::CameraComponent oCamComponent(oCamDesc);
	Bta::Graphic::TransformComponentGPU oGPUCam;
	pEntityCam->AddExistingComponent(&oCamComponent);
	pEntityCam->AddExistingComponent(&oGPUCam);

	Bta::Graphic::RenderBatch::Desc oDesc;
	oDesc.iSampleCount = VK_SAMPLE_COUNT_1_BIT;
	oDesc.bPresentable = false;
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
	Bta::Graphic::MaterialComponent* pMat = oRenderBatch.GetSubBatches()[0]->AddMesh(&oMeshComponent);
	pMat->CommitChange();

	Bta::Graphic::Globals::g_pOutput->GenerateFramebuffers({ Bta::Graphic::Globals::g_pOutput->GetSwapchain()->GetFormat(),VK_FORMAT_D32_SFLOAT }, &oRenderBatch);

	Bta::Graphic::ImGuiWrapper::Desc oImDesc;
	oImDesc.pCallback = [&](void* pData) { ImGui::Text("Sample text"); };
	Bta::Graphic::Globals::g_pImGui = new Bta::Graphic::ImGuiWrapper(oImDesc);

	while (!glfwWindowShouldClose(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow()))
	{
		glfwPollEvents();

		int state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_UP);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(0, 0, 0.001f), true);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_DOWN);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(0, 0, -0.001f), true);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_LEFT);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(0.001f, 0, 0), true);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_RIGHT);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetPosition(glm::vec3(-0.001f, 0, 0), true);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_SPACE);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetRotation(glm::vec3(1, 0, 0), 0.001f, true, false);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_O);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetRotation(glm::vec3(0,1,0), 0.001f, true, true);
			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		state = glfwGetKey(Bta::Graphic::Globals::g_pOutput->GetRenderSurface()->GetModifiableWindow(), GLFW_KEY_P);
		if (state == GLFW_PRESS)
		{
			pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->SetForward(glm::normalize(glm::vec3(1, 1, 1)));

			glm::vec3 vLocalDirection = pEntity->FindFirstComponent<Bta::Core::TransformComponent>()->GetLocalDirection(glm::vec3(0, 0, 1));
			std::cout << vLocalDirection.x << " / " << vLocalDirection.y << " / " << vLocalDirection.z << std::endl;

			pEntity->FindFirstComponent < Bta::Graphic::TransformComponentGPU>()->RefreshGPUMemory();
		}

		Bta::Graphic::Globals::g_pOutput->RenderOneFrame({ &oRenderBatch }, true);
		Bta::Graphic::Globals::g_pOutput->Present();
		Bta::Graphic::Globals::g_pOutput->NextFrame();
	}

	return 0;
}