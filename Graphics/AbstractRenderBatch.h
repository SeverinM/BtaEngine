#ifndef H_ABSTRACT_BATCH
#define H_ABSTRACT_BATCH
#include <string>
#include "vulkan/vulkan_core.h"

class Framebuffer;

class AbstractRenderBatch
{
public:
	virtual VkCommandBuffer* GetDrawCommand(Framebuffer* pFramebuffer) = 0;

	inline bool IsEnabled() { return m_bEnabled; }
	inline void SetEnabled(bool bValue) { m_bEnabled = bValue; MarkAsDirty(); }
	virtual void MarkAsDirty() = 0;

protected:
	AbstractRenderBatch* m_pNextBatch;
	virtual void ChainSubpass(VkCommandBuffer* pCommand) = 0;
	std::string m_sTag;
	bool m_bEnabled;
};

#endif

