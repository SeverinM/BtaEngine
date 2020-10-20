#ifndef H_MODEL
#define H_MODEL
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/glm.hpp"
#include "GLM/gtx/hash.hpp"
#include "Buffer.h"
#include <string>
#include <unordered_map>

class Model
{
public:
	struct Desc
	{
		std::string sTexturepath;
		std::string sModelPath;
		CommandFactory* pFactory;
		GraphicWrapper* pWrapper;
		bool bEnableMip;
	};

	struct Vertex
	{
		glm::vec3 vPos;
		glm::vec2 vTex;

		bool operator==(const Vertex& oOther) const
		{
			return oOther.vPos == vPos && oOther.vTex == vTex;
		}
	};

	struct BuffersVertices
	{
		BasicBuffer* pVertices;
		BasicBuffer* pIndices;
	};

	enum eVerticesAttributes
	{
		E_POSITIONS,
		E_NORMALS,
		E_COLOR,
		E_UV
	};

	Model(Desc& oDesc);
	BuffersVertices ConvertToBuffer(std::unordered_map<eVerticesAttributes, size_t> oMemoryLayout, GraphicWrapper* pWrapper, CommandFactory* pFactory);
	Image* GetTexture() { return m_pTexture; }

private:
	std::vector<Vertex> m_oVertices;
	std::vector<uint32_t> m_oIndices;
	Image* m_pTexture;
};

#endif H_MODEL

