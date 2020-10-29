#ifndef H_TRANSFORM
#define H_TRANSFORM
#include "GLM/glm.hpp"
#include "GLM/ext/matrix_transform.hpp"
#include "Buffer.h"

class Transform
{
protected:
	glm::mat4 m_mModel;
public:
	Transform(glm::mat4& mInitialModel);
	Transform();
	virtual void SetPosition(glm::vec3 vNewPosition, bool bRelative = false);
	virtual void SetScale(glm::vec3 vNewScale, bool bRelative = false);
	virtual void Rotate(glm::vec3 vAxis, float fDegrees);
	virtual void ForceMatrix(glm::mat4 mMatrix);
	glm::vec3 GetForward() const;
	glm::vec3 GetUp() const;
	glm::vec3 GetRight() const;
	glm::vec3 GetPosition() const;
	inline glm::mat4 GetModelMatrix() const{ return m_mModel;}
};

class BufferedTransform : public Transform
{
public:
	BufferedTransform(glm::mat4& mInitialMode, uint64_t iOffset, std::shared_ptr<Buffer> xBuffer, GraphicDevice* pDevice);

	void SetPosition(glm::vec3 vNewPosition, bool bRelative = false) override;
	void SetScale(glm::vec3 vNewScale, bool bRelative = false) override;
	void Rotate(glm::vec3 vAxis, float fDegrees) override;
	void ForceMatrix(glm::mat4 mMatrix) override;
	static std::vector<BufferedTransform*> MergeTransform(std::vector<std::shared_ptr<Transform>> oTrsf, VkBufferUsageFlags eUsage, GraphicWrapper* pWrapper);

	protected:
		uint64_t m_iOffset;
	 	std::shared_ptr<Buffer> m_xBuffer;
		void UpdateBuffer();
		GraphicDevice* m_pDevice;
};


#endif