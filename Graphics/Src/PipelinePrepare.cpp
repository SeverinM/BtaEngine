#include "PipelinePrepare.h"

SkyboxPrepare::SkyboxPrepare(Desc& oDesc)
{
	m_pFilenames = oDesc.sFilenames;
	m_vOriginalRotation = oDesc.vOriginalRotation;
}
