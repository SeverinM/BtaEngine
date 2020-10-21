#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <functional>
#include <iostream>

namespace std
{
	template<> struct hash<Model::Vertex>
	{
		size_t operator()(Model::Vertex const& oVertex) const
		{
			size_t iHashPos = hash<glm::vec3>{}(oVertex.vPos);
			size_t iHashTex = hash<glm::vec2>{}(oVertex.vTex);

			return ( iHashPos ^ ( (iHashTex << 1 ) >> 1 ) /*^ ( iHashNormals << 1 )*/ );
		}
	};
}

Model::Model(Desc& oDesc)
{
	Image::FromFileDesc oTexDesc;
	oTexDesc.bEnableMip = oDesc.bEnableMip;
	oTexDesc.eAspect = VK_IMAGE_ASPECT_COLOR_BIT;
	oTexDesc.eFormat = VK_FORMAT_R8G8B8A8_SRGB;
	oTexDesc.eSampleFlag = VK_SAMPLE_COUNT_1_BIT;
	oTexDesc.eTiling = VK_IMAGE_TILING_OPTIMAL;
	oTexDesc.pFactory = oDesc.pFactory;
	oTexDesc.pWrapper = oDesc.pWrapper;

	m_pTexture = oDesc.sTexturepath.empty() ? nullptr : Image::CreateFromFile(oDesc.sTexturepath, oTexDesc);

	tinyobj::attrib_t oAttrib;
	std::vector<tinyobj::shape_t> oShapes;
	std::vector<tinyobj::material_t> oMaterials;
	std::string sWarn, sErr;

	if (!tinyobj::LoadObj(&oAttrib, &oShapes, &oMaterials, &sWarn, &sErr, oDesc.sModelPath.c_str()))
	{
		throw std::runtime_error("Could not load model");
	}

	std::unordered_map<Vertex, uint32_t> oUniqueVertices;

	for (const auto& oShape : oShapes)
	{
		for (const auto& oIndex : oShape.mesh.indices)
		{
			Vertex oVertex{};
			oVertex.vPos = { oAttrib.vertices[3 * oIndex.vertex_index + 0], oAttrib.vertices[3 * oIndex.vertex_index + 1], oAttrib.vertices[3 * oIndex.vertex_index + 2] };
			oVertex.vTex = { oAttrib.texcoords[2 * oIndex.texcoord_index + 0], 1.0f - oAttrib.texcoords[2 * oIndex.texcoord_index + 1] };

			if (oUniqueVertices.count(oVertex) == 0)
			{
				oUniqueVertices[oVertex] = m_oVertices.size();
				m_oVertices.push_back(oVertex);
			}

			m_oIndices.push_back(oUniqueVertices[oVertex]);
		}
	}
}

Model::BuffersVertices Model::ConvertToBuffer(std::unordered_map<eVerticesAttributes, size_t> oMemoryLayout, GraphicWrapper* pWrapper, CommandFactory* pFactory)
{
	Model::BuffersVertices oOutput;

	//vertices ( uniques )
	BasicBuffer::Desc oDesc;
	oDesc.eUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	oDesc.pWrapper = pWrapper;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	oDesc.iUnitCount = m_oVertices.size();
	oDesc.iUnitSize = oMemoryLayout[E_POSITIONS] + oMemoryLayout[E_NORMALS] + oMemoryLayout[E_COLOR] + oMemoryLayout[E_UV];

	BasicBuffer* pBuffer = new BasicBuffer(oDesc);
	void* pOutput = m_oVertices.data();
	
	pBuffer->CopyFromMemory(pOutput, pWrapper->GetModifiableDevice());
	
	oDesc.eUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	oOutput.pVertices = new BasicBuffer(oDesc);
	pBuffer->SendCopyCommand(oOutput.pVertices, pFactory);

	//Indices
	oDesc.eUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	oDesc.iUnitSize = sizeof(size_t);
	oDesc.iUnitCount = m_oIndices.size();

	pBuffer = new BasicBuffer(oDesc);
	pBuffer->CopyFromMemory(m_oIndices.data(), pWrapper->GetModifiableDevice());

	oDesc.eUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	oDesc.oPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	oOutput.pIndices = new BasicBuffer(oDesc);
	pBuffer->SendCopyCommand(oOutput.pIndices, pFactory);

	return oOutput;
}
