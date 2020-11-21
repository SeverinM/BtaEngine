#include "Transform.h"
#include <iostream>
#include "GLM/gtx/string_cast.hpp"
#include "Globals.h"

Transform::Transform(glm::mat4& mInitialModel) : m_xParent(nullptr)
{
	m_mModel = mInitialModel;
}

Transform::Transform()
{
	m_mModel = glm::mat4(1.0f);
}

void Transform::SetPosition(glm::vec3 vNewPosition, bool bRelative /*= false*/)
{
	glm::vec3 vPos = GetPosition();
	if (!bRelative)
	{
		m_mModel[3][0] = 0.0f;
		m_mModel[3][1] = 0.0f;
		m_mModel[3][2] = 0.0f;
	}

	m_mModel = glm::translate(m_mModel, vNewPosition);

	for (std::shared_ptr< Transform> xChild : m_oChilds)
	{
		xChild->SetPosition(GetPosition() - vPos , true);
	}
}

void Transform::SetScale(glm::vec3 vNewScale, bool bRelative /*= false*/)
{
	m_mModel = glm::scale(m_mModel, vNewScale);
}

void Transform::Rotate(glm::vec3 vAxis, float fDegrees)
{
	m_mModel = glm::rotate(m_mModel, glm::radians(fDegrees), vAxis);
}

void Transform::ForceMatrix(glm::mat4 mMatrix)
{
	m_mModel = mMatrix;
}

glm::vec3 Transform::GetForward() const
{
	return glm::vec3(m_mModel[0][2], m_mModel[1][2], m_mModel[2][2]);
}

glm::vec3 Transform::GetUp() const
{
	return glm::vec3(m_mModel[0][1], m_mModel[1][1], m_mModel[2][1]);
}

glm::vec3 Transform::GetRight() const
{
	return glm::vec3(m_mModel[0][0], m_mModel[1][0], m_mModel[2][0]);
}

glm::vec3 Transform::GetPosition() const
{
	return glm::vec3(m_mModel[3][0], m_mModel[3][1], m_mModel[3][2]);
}

void Transform::SetParent(std::shared_ptr<Transform> xParent)
{
	if (m_xParent != nullptr)
	{
		std::vector< std::shared_ptr<Transform>>::iterator pIt = std::find(m_xParent->m_oChilds.begin(), m_xParent->m_oChilds.end(), std::shared_ptr<Transform>(this));
		if (pIt != m_xParent->m_oChilds.end())
			xParent->m_oChilds.erase(pIt);
	}

	m_xParent = xParent;

	if (xParent != nullptr)
	{
		xParent->m_oChilds.push_back(std::shared_ptr<Transform>(this));
	}
}

void Transform::AddChild(std::shared_ptr<Transform> xChild)
{
	m_oChilds.push_back(xChild);
	xChild->m_xParent = std::shared_ptr<Transform>(this);
}

BufferedTransform::BufferedTransform(glm::mat4& mInitialMode, uint64_t iOffset, std::shared_ptr<Buffer> xBuffer, GraphicDevice* pDevice) : Transform(mInitialMode)
{
	m_iOffset = iOffset;
	m_xBuffer = xBuffer;
}

void BufferedTransform::SetPosition(glm::vec3 vNewPosition, bool bRelative /*= false*/)
{
	Transform::SetPosition(vNewPosition, bRelative);

	for (std::shared_ptr< Transform> xChild : m_oChilds)
	{
		if (std::dynamic_pointer_cast<BufferedTransform>(xChild) != nullptr)
		{
			std::dynamic_pointer_cast<BufferedTransform>(xChild)->UpdateBuffer();
		}
	}

	UpdateBuffer();
}

void BufferedTransform::SetScale(glm::vec3 vNewScale, bool bRelative /*= false*/)
{
	Transform::SetScale(vNewScale, bRelative);
	UpdateBuffer();
}

void BufferedTransform::Rotate(glm::vec3 vAxis, float fDegrees)
{
	Transform::Rotate(vAxis, fDegrees);
	UpdateBuffer();
}

void BufferedTransform::ForceMatrix(glm::mat4 mMatrix)
{
	Transform::ForceMatrix(mMatrix);
	UpdateBuffer();
}

std::shared_ptr<BasicBuffer> BufferedTransform::MergeTransform(std::vector<std::shared_ptr<Transform>> oTrsf, VkBufferUsageFlags eUsage, GraphicWrapper* pWrapper, std::vector<std::shared_ptr<BufferedTransform>>& oBufferedTransform)
{
	oBufferedTransform.clear();
	BasicBuffer::Desc oDesc;
	oDesc.eUsage = eUsage;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	oDesc.iUnitSize = sizeof(glm::mat4);
	oDesc.iUnitCount = (uint32_t)oTrsf.size();
	std::shared_ptr<BasicBuffer> xBuffer = std::shared_ptr<BasicBuffer>( new BasicBuffer(oDesc) );
	uint64_t iOffset = 0;

	for (std::shared_ptr<Transform> xTrans : oTrsf)
	{
		glm::mat4 mInput = xTrans->GetModelMatrix();
		oBufferedTransform.push_back(std::make_shared<BufferedTransform> (BufferedTransform( mInput, iOffset, std::static_pointer_cast<Buffer>(xBuffer), Graphics::Globals::g_pDevice)));
		iOffset += sizeof(glm::mat4);
		oBufferedTransform[oBufferedTransform.size() - 1]->UpdateBuffer();
	}

	return xBuffer;
}

void BufferedTransform::UpdateBuffer()
{
	void* pData = &m_mModel;
	m_xBuffer->CopyFromMemory(pData, Graphics::Globals::g_pDevice, m_iOffset, sizeof(glm::mat4));
}
