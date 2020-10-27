#include "Camera.h"
#include "GLM/ext/matrix_clip_space.hpp"
#include <iostream>
#include "GLM/gtx/string_cast.hpp"

Camera::Camera(Desc& oDesc)
{
	m_mViewMatrix = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

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
	oBufferDesc.pWrapper = oDesc.pWrapper;
	oBufferDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	m_pMatriceBuffer = new BasicBuffer(oBufferDesc);

	std::vector<glm::mat4> oVP = { m_mViewMatrix, m_mProjectionMatrix };
	m_pMatriceBuffer->CopyFromMemory(oVP.data(), oDesc.pWrapper->GetModifiableDevice());
}

Camera::~Camera()
{
	delete m_pMatriceBuffer;
}

glm::vec3 Camera::GetForward()
{
	return glm::vec3(m_mViewMatrix[0][2], m_mViewMatrix[1][2], m_mViewMatrix[2][2]);
}

glm::vec3 Camera::GetRight()
{
	return glm::vec3(m_mViewMatrix[0][0], m_mViewMatrix[1][0], m_mViewMatrix[2][0]);
}

glm::vec3 Camera::GetUp()
{
	return glm::vec3(m_mViewMatrix[0][1], m_mViewMatrix[1][1], m_mViewMatrix[2][1]);
}

void Camera::Translate(glm::vec3 vNewPos)
{
	m_mViewMatrix = glm::translate(m_mViewMatrix, vNewPos);
}

void Camera::Rotate(glm::vec3 vAxis, float fDegreeAngle)
{
	m_mViewMatrix = glm::rotate(m_mViewMatrix, glm::radians(fDegreeAngle), vAxis);
}

