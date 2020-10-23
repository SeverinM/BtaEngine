#include "Camera.h"
#include "GLM/ext/matrix_clip_space.hpp"
#include <iostream>

Camera::Camera(Desc& oDesc)
{
	m_pTransform = oDesc.pTransform;

	if (oDesc.fNearPlane > oDesc.fFarPlane || oDesc.fNearPlane <= 0.0f)
	{
		throw std::runtime_error("Cannot create camera with such parameters");
	}

	m_mProjectionMatrix = glm::perspective(glm::radians(oDesc.fAngleDegree), oDesc.fRatio, oDesc.fNearPlane, oDesc.fFarPlane);
}

