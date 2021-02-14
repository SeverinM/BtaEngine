#include "Camera.h"
#include "GLM/ext/matrix_clip_space.hpp"
#include <iostream>
#include "GLM/gtx/string_cast.hpp"
#include "Globals.h"

namespace Bta
{
	namespace Graphic
	{
		Camera::Camera(Desc& oDesc)
		{
			m_fMoveSpeed = 1.0f;
			m_fAngularSpeed = 50.0f;

			if (oDesc.fNearPlane > oDesc.fFarPlane || oDesc.fNearPlane <= 0.0f)
			{
				throw std::runtime_error("Cannot create camera with such parameters");
			}

			m_mProjectionMatrix = glm::perspective(glm::radians(oDesc.fAngleDegree), oDesc.fRatio, oDesc.fNearPlane, oDesc.fFarPlane);
			m_mProjectionMatrix[1][1] *= -1;

			BasicBuffer::Desc oBufferDesc;
			oBufferDesc.eUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			oBufferDesc.iUnitSize = sizeof(glm::mat4);
			oBufferDesc.iUnitCount = 2;
			oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			m_xMatriceBuffer = std::shared_ptr<BasicBuffer>( GPUMemory::GetInstance()->AllocateMemory(oBufferDesc));

			m_pTransform = new BufferedTransform(oDesc.mInitialMatrix, 0, m_xMatriceBuffer, Bta::Graphic::Globals::g_pDevice);

			std::vector<glm::mat4> oVP = { m_pTransform->GetModelMatrix(), m_mProjectionMatrix };
			m_xMatriceBuffer->CopyFromMemory(oVP.data(), Bta::Graphic::Globals::g_pDevice);

		}
	}
}

