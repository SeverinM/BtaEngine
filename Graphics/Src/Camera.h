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
		Transform* pTransform;
	};
	Camera(Desc& oDesc);
	inline glm::mat4 GetProjectionMatrix() { return m_mProjectionMatrix; }
	inline glm::mat4 GetViewMatrix() { return m_pTransform->GetModelMatrix(); }

private:
	Transform* m_pTransform;
	glm::mat4 m_mProjectionMatrix;
};

#endif