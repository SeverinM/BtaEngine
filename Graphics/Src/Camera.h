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
	};
	Camera(Desc& oDesc);

	inline glm::mat4 GetProjectionMatrix() { return m_mProjectionMatrix; }
	inline glm::mat4 GetViewMatrix() { return m_mViewMatrix; }
	inline std::shared_ptr<BasicBuffer> GetVPMatriceBuffer() { return m_xMatriceBuffer; }
	inline glm::mat4& GetModifiableMatrixView() { return m_mViewMatrix; }
	glm::vec3 GetForward();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetPosition();
	void Translate(glm::vec3 vNewPos);
	void Rotate(glm::vec3 vAxis, float fDegreeAngle);

private:
	glm::mat4 m_mViewMatrix;
	glm::mat4 m_mProjectionMatrix;
	std::shared_ptr<BasicBuffer> m_xMatriceBuffer;
};

#endif