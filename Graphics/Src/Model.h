#ifndef H_MODEL
#define H_MODEL
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/glm.hpp"
#include "GLM/gtx/hash.hpp"
#include "GLM/ext/matrix_transform.hpp"
#include "Buffer.h"
#include <string>
#include <unordered_map>
#include "Transform.h"

class RenderModel
{
public:

	typedef uint32_t BufferElementsFlag;
	enum eVerticesAttributes
	{
		E_POSITIONS = 1 << 0,
		E_NORMALS = 1 << 1,
		E_COLOR = 1 << 2,
		E_UV = 1 << 3
	};

	struct Desc
	{
		std::vector<glm::vec4> oColors;
		std::vector<glm::vec3> oPositions;
		std::vector<glm::vec2> oUVs;
		std::vector<glm::vec3> oNormals;
		std::vector< uint32_t > oIndexes;
		std::vector<std::string> oFilenamesTextures;
		std::string sFilenameModel;
		std::vector<Transform*> oModels;
		GraphicWrapper* pWrapper;
		CommandFactory* pFactory;
		BufferElementsFlag eFlag;
	};
	RenderModel(Desc& oDesc);
	void ConvertToBuffer(BufferElementsFlag eFlags, bool bIncludeIndexes, GraphicWrapper* pWrapper);
	inline Buffer* GetVerticesBuffer() { return m_pCachedVerticesBuffer; }
	inline Buffer* GetIndexesBuffer() { return m_pCachedIndexesBuffer; }
	inline Buffer* GetModelMatrices() { return m_pAllModelMatrices; }
	inline std::vector<Image*>& GetTextures() { return m_oTextures; }
	inline std::vector<Transform*> GetModels() { return m_oModels; }
	BufferElementsFlag GetBufferFlags();

	protected:
		void LoadModel(Desc& oDesc);
		std::vector<glm::vec4> m_oColors;
		std::vector<glm::vec3> m_oPositions;
		std::vector<glm::vec2> m_oUVs;
		std::vector<glm::vec3> m_oNormals;
		std::vector <uint32_t> m_oIndexes;
		std::vector<Image*> m_oTextures;

		std::vector<Transform*> m_oModels;
		Buffer* m_pAllModelMatrices;
		int m_iInstanceCount;

		Buffer* m_pCachedVerticesBuffer;
		Buffer* m_pCachedIndexesBuffer;
};

#endif H_MODEL

