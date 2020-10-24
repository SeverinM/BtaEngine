#ifndef H_CAMERA
#define H_CAMERA
#include "Transform.h"

class Camera
{
public:
	struct Desc
	{
		float fNearPlane;
		float fFarPlane;
		float fAngleDegree;
		float fRatio;
	};
	Camera(Desc& oDesc);
	inline glm::mat4 GetProjectionMatrix() { return m_mProjectionMatrix; }
	inline glm::mat4 GetViewMatrix() { return m_mViewMatrix; }

private:
	glm::mat4 m_mViewMatrix;
	glm::mat4 m_mProjectionMatrix;
};

#endif