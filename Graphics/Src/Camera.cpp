#include "Camera.h"
#include "GLM/ext/matrix_clip_space.hpp"
#include <iostream>

Camera::Camera(Desc& oDesc)
{
	m_mViewMatrix = glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	if (oDesc.fNearPlane > oDesc.fFarPlane || oDesc.fNearPlane <= 0.0f)
	{
		throw std::runtime_error("Cannot create camera with such parameters");
	}

	m_mProjectionMatrix = glm::perspective(glm::radians(oDesc.fAngleDegree), oDesc.fRatio, oDesc.fNearPlane, oDesc.fFarPlane);
	m_mProjectionMatrix[1][1] *= -1;
}

