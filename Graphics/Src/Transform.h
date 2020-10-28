#ifndef H_TRANSFORM
#define H_TRANSFORM
#include "GLM/glm.hpp"
#include "GLM/ext/matrix_transform.hpp"

class Transform
{
private:
	glm::mat4 m_mModel;
public:
	Transform(glm::mat4& mInitialModel);
	Transform();
	void SetPosition(glm::vec3 vNewPosition, bool bRelative = false);
	void SetScale(glm::vec3 vNewScale, bool bRelative = false);
	void Rotate(glm::vec3 vAxis, float fDegrees);
	glm::vec3 GetForward() const;
	glm::vec3 GetUp() const;
	glm::vec3 GetRight() const;
	glm::vec3 GetPosition() const;
	inline const glm::mat4 GetModelMatrix(){ return m_mModel;}
};


#endif