#ifndef H_CAMERA
#define H_CAMERA
#include "Transform.h"
#include "Buffer.h"
#include "GraphicWrapper.h"

class Camera
{
public:
	struct Desc
	{
		float fNearPlane;
		float fFarPlane;
		float fAngleDegree;
		float fRatio;
		GraphicWrapper* pWrapper;
		glm::mat4 mInitialMatrix;
	};
	Camera(Desc& oDesc);

	inline glm::mat4 GetProjectionMatrix() { return m_mProjectionMatrix; }
	inline glm::mat4 GetViewMatrix() { return m_pTransform->GetModelMatrix(); }
	inline std::shared_ptr<BasicBuffer> GetVPMatriceBuffer() { return m_xMatriceBuffer; }
	BufferedTransform* GetTransform() { return m_pTransform; }

private:
	BufferedTransform* m_pTransform;
	glm::mat4 m_mProjectionMatrix;
	std::shared_ptr<BasicBuffer> m_xMatriceBuffer;
};

#endif