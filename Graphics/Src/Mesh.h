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
#include <memory>

class Mesh
{
public:

	typedef std::shared_ptr<Mesh> StrongPtr;
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
		std::string sFilenameModel;
		std::vector< std::shared_ptr< Transform >> oModels;
		GraphicWrapper* pWrapper;
		CommandFactory* pFactory;
		BufferElementsFlag eFlag;
	};

	Mesh(Desc& oDesc);
	~Mesh();

	void ConvertToVerticesBuffer(BufferElementsFlag eFlags, bool bIncludeIndexes, GraphicWrapper* pWrapper);
	inline std::shared_ptr<Buffer> GetVerticesBuffer() { return m_xCachedVerticesBuffer; }
	inline std::shared_ptr<Buffer> GetIndexesBuffer() { return m_xCachedIndexesBuffer; }
	inline std::shared_ptr<BasicBuffer> GetModelMatrices() { return m_xAllModelMatrices; }
	inline std::vector< std::shared_ptr<BufferedTransform>> GetModels() { return m_oModels; }
	inline size_t GetVerticeCount() { return m_oPositions.size(); }
	inline size_t GetInstanceCount() { return m_oModels.size(); }
	void SetTransforms(std::vector< std::shared_ptr<BufferedTransform>> oValues, GraphicWrapper* pWrapper)
	{
		std::vector<std::shared_ptr< Transform >> oTrsfs;
		for (std::shared_ptr<BufferedTransform> xTrsf : oValues)
		{
			oTrsfs.push_back(std::static_pointer_cast<Transform>(xTrsf));
		}

		m_xAllModelMatrices = BufferedTransform::MergeTransform(oTrsfs, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, pWrapper, m_oModels);
	}
	inline std::vector< std::shared_ptr<BufferedTransform>> GetTransforms() { return m_oModels; }

	BufferElementsFlag GetBufferFlags();

	protected:
		void LoadModel(Desc& oDesc);
		std::vector<glm::vec4> m_oColors;
		std::vector<glm::vec3> m_oPositions;
		std::vector<glm::vec2> m_oUVs;
		std::vector<glm::vec3> m_oNormals;
		std::vector <uint32_t> m_oIndexes;

		std::vector< std::shared_ptr<BufferedTransform>> m_oModels;
		std::shared_ptr< BasicBuffer > m_xAllModelMatrices;

		std::shared_ptr<Buffer> m_xCachedVerticesBuffer;
		std::shared_ptr<Buffer> m_xCachedIndexesBuffer;
};

#endif H_MODEL

