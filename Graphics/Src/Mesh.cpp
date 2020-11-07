#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <functional>
#include <iostream>

Mesh::Mesh(Desc& oDesc)
{
	//Load textures
	Image::FromFileDesc oTexDesc;
	oTexDesc.bEnableMip = true;
	oTexDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oTexDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
	oTexDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
	oTexDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
	oTexDesc.pFactory = oDesc.pFactory;
	oTexDesc.pWrapper = oDesc.pWrapper;

	//Load vertices
	if (oDesc.oPositions.size() > 0)
	{
		if ( oDesc.eFlag & eVerticesAttributes::E_POSITIONS)
			m_oPositions = oDesc.oPositions;

		if (oDesc.eFlag & eVerticesAttributes::E_COLOR)
			m_oColors = oDesc.oColors;
		
		if ( oDesc.eFlag & eVerticesAttributes::E_UV)
			m_oUVs = oDesc.oUVs;

		if (oDesc.eFlag & eVerticesAttributes::E_NORMALS)
			m_oNormals = oDesc.oNormals;

		m_oIndexes = oDesc.oIndexes;
		m_oNormals = oDesc.oNormals;
	}
	else
	{
		if (oDesc.sFilenameModel.empty())
		{
			throw std::runtime_error("Invalid model file");
		}
		LoadModel(oDesc);
	}

	m_xAllModelMatrices = BufferedTransform::MergeTransform(oDesc.oModels, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, oDesc.pWrapper, m_oModels);
	for (int i = 0; i < m_oModels.size(); i++)
	{
		m_oModels[i]->ForceMatrix(oDesc.oModels[i]->GetModelMatrix());
	}
}

Mesh::~Mesh()
{
	m_oColors.clear();
	m_oIndexes.clear();
	m_oNormals.clear();
	m_oUVs.clear();
	m_oPositions.clear();
	m_oModels.clear();
}

void Mesh::ConvertToVerticesBuffer(BufferElementsFlag eFlags, bool bIncludeIndexes, GraphicWrapper* pWrapper)
{
	std::vector<uint8_t> oBytes;
	int iUnitSize = 0;

	if (eFlags & eVerticesAttributes::E_COLOR)
	{
		iUnitSize += sizeof(glm::vec4);
	}

	if (eFlags & eVerticesAttributes::E_NORMALS)
	{
		iUnitSize += sizeof(glm::vec3);
	}

	if (eFlags & eVerticesAttributes::E_POSITIONS)
	{
		iUnitSize += sizeof(glm::vec3);
	}

	if (eFlags & eVerticesAttributes::E_UV)
	{
		iUnitSize += sizeof(glm::vec2);
	}

	oBytes.resize(iUnitSize * m_oPositions.size());

	int iOffset = 0;
	for (int i = 0; i < m_oPositions.size(); i++)
	{
		if (eFlags & eVerticesAttributes::E_POSITIONS)
		{
			memcpy(oBytes.data() + iOffset, &m_oPositions[i], sizeof(glm::vec3));
			iOffset += sizeof(glm::vec3);
		}

		if (eFlags & eVerticesAttributes::E_UV)
		{
			memcpy(oBytes.data() + iOffset, &m_oUVs[i], sizeof(glm::vec2));
			iOffset += sizeof(glm::vec2);
		}

		if (eFlags & eVerticesAttributes::E_NORMALS)
		{
			memcpy(oBytes.data() + iOffset, &m_oNormals[i], sizeof(glm::vec3));
			iOffset += sizeof(glm::vec3);
		}

		if (eFlags & eVerticesAttributes::E_COLOR)
		{
			memcpy(oBytes.data() + iOffset, &m_oColors[i], sizeof(glm::vec4));
			iOffset += sizeof(glm::vec4);
		}
	}

	BasicBuffer::Desc oDesc;
	oDesc.iUnitSize = iUnitSize;
	oDesc.iUnitCount = m_oPositions.size();
	oDesc.eUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	oDesc.pWrapper = pWrapper;

	m_xCachedVerticesBuffer = std::shared_ptr<Buffer>( new BasicBuffer(oDesc) );
	m_xCachedVerticesBuffer->CopyFromMemory(oBytes.data(), pWrapper->GetModifiableDevice());

	if (bIncludeIndexes)
	{
		BasicBuffer::Desc oDescIndex;
		oDescIndex.iUnitSize = sizeof(uint32_t);
		oDescIndex.iUnitCount = m_oIndexes.size();
		oDescIndex.pWrapper = pWrapper;
		oDescIndex.eUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		oDescIndex.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		m_xCachedIndexesBuffer = std::shared_ptr<Buffer>( new BasicBuffer(oDescIndex) );
		m_xCachedIndexesBuffer->CopyFromMemory(m_oIndexes.data(), pWrapper->GetModifiableDevice());
	}
}

Mesh::BufferElementsFlag Mesh::GetBufferFlags()
{
	BufferElementsFlag eFlag = 0;
	eFlag |= (m_oPositions.size() == 0 ? 0 : eVerticesAttributes::E_POSITIONS);
	eFlag |= (m_oNormals.size() == 0 ? 0 : eVerticesAttributes::E_NORMALS);
	eFlag |= (m_oUVs.size() == 0 ? 0 : eVerticesAttributes::E_UV);
	eFlag |= (m_oColors.size() == 0 ? 0 : eVerticesAttributes::E_COLOR);
	return eFlag;
}

void Mesh::LoadModel(Desc& oDesc)
{
	tinyobj::attrib_t oAttrib;
	std::vector<tinyobj::shape_t> oShapes;
	std::vector<tinyobj::material_t> oMaterials;
	std::string sWarn, sErr;

	if (!tinyobj::LoadObj(&oAttrib, &oShapes, &oMaterials, &sWarn, &sErr, oDesc.sFilenameModel.c_str()))
	{
		throw std::runtime_error("Could not load model");
	}

	std::unordered_map<size_t, uint32_t> oUniqueVertices;

	for (const auto& oShape : oShapes)
	{
		for (const auto& oIndex : oShape.mesh.indices)
		{
			glm::vec3 vPos = glm::vec3(0.0f);
			glm::vec2 vTex = glm::vec2(0.0f);
			glm::vec3 vNormal = glm::vec3(0.0f);
			
			if (oIndex.vertex_index >= 0)
			{
				vPos = { oAttrib.vertices[3 * oIndex.vertex_index + 0], oAttrib.vertices[3 * oIndex.vertex_index + 1], oAttrib.vertices[3 * oIndex.vertex_index + 2] };
			}

			if (oIndex.texcoord_index >= 0)
			{
				vTex = { oAttrib.texcoords[2 * oIndex.texcoord_index + 0], 1.0f - oAttrib.texcoords[2 * oIndex.texcoord_index + 1] };
			}

			if (oIndex.normal_index >= 0)
			{
				vNormal = { oAttrib.normals[3 * oIndex.normal_index + 0], oAttrib.vertices[3 * oIndex.normal_index + 1], oAttrib.vertices[3 * oIndex.normal_index + 2] };
			}

			size_t iHash = std::hash<glm::vec3>()(vPos) ^ (std::hash<glm::vec2>()(vTex) << 1);
			if (oUniqueVertices.count(iHash) == 0)
			{
				oUniqueVertices[iHash] = m_oPositions.size();

				if ( oDesc.eFlag & eVerticesAttributes::E_POSITIONS)
					m_oPositions.push_back(vPos);

				if ( oDesc.eFlag & eVerticesAttributes::E_NORMALS)
					m_oNormals.push_back(vNormal);

				if ( oDesc.eFlag & eVerticesAttributes::E_UV)
					m_oUVs.push_back(vTex);
			}

			m_oIndexes.push_back(oUniqueVertices[iHash]);
		}
	}
}
