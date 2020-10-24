#ifndef H_PIPELINE_PREPARE
#define H_PIPELINE_PREPARE
#include "GLM/glm.hpp"
#include "Buffer.h"

class PipelinePrepare
{
protected :
	PipelinePrepare() {};
};

class SkyboxPrepare : public PipelinePrepare
{
public:
	struct Desc
	{
		std::string sFilenames[6];
		glm::vec3 vOriginalRotation;
	};
	SkyboxPrepare(Desc& oDesc);

protected:
	std::string* m_pFilenames;
	glm::vec3 m_vOriginalRotation;
};

class MainRenderPrepare : public PipelinePrepare
{

};

#endif H_PIPELINE_PREPARE

