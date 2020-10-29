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
		std::vector< std::shared_ptr< Transform >> oModels;
		GraphicWrapper* pWrapper;
		CommandFactory* pFactory;
		BufferElementsFlag eFlag;
	};

	RenderModel(Desc& oDesc);
	~RenderModel();

	void ConvertToBuffer(BufferElementsFlag eFlags, bool bIncludeIndexes, GraphicWrapper* pWrapper);
	inline std::shared_ptr<Buffer> GetVerticesBuffer() { return m_xCachedVerticesBuffer; }
	inline std::shared_ptr<Buffer> GetIndexesBuffer() { return m_xCachedIndexesBuffer; }
	inline std::shared_ptr<Buffer> GetModelMatrices() { return m_xAllModelMatrices; }
	inline std::vector< std::shared_ptr<Image>>& GetTextures() { return m_oTextures; }
	inline std::vector< std::shared_ptr<BufferedTransform>>& GetModels() { return m_oModels; }
	inline uint64_t GetVerticeCount() { return m_iVerticesCount; }
	inline int GetInstanceCount() { return m_iInstanceCount; }
	inline VkDescriptorSet* GetDescriptorSet() { return m_pDescriptorSet; }
	inline void SetDescriptorSet(VkDescriptorSet* pDescriptorSet) { m_pDescriptorSet = pDescriptorSet; }

	BufferElementsFlag GetBufferFlags();

	protected:
		uint64_t m_iVerticesCount;
		void LoadModel(Desc& oDesc);
		std::vector<glm::vec4> m_oColors;
		std::vector<glm::vec3> m_oPositions;
		std::vector<glm::vec2> m_oUVs;
		std::vector<glm::vec3> m_oNormals;
		std::vector <uint32_t> m_oIndexes;
		std::vector< std::shared_ptr< Image >> m_oTextures;

		std::vector< std::shared_ptr<BufferedTransform>> m_oModels;
		std::shared_ptr< Buffer > m_xAllModelMatrices;
		int m_iInstanceCount;

		std::shared_ptr<Buffer> m_xCachedVerticesBuffer;
		std::shared_ptr<Buffer> m_xCachedIndexesBuffer;
		VkDescriptorSet* m_pDescriptorSet;
};

#endif H_MODEL

