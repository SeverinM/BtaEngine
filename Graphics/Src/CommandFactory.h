#ifndef H_COMMAND_FACTORY
#define H_COMMAND_FACTORY
#include "Framebuffer.h"
#include "Pipeline.h"

namespace Bta
{
	namespace Graphic
	{
		class Buffer;

		class CommandFactory
		{
		public:
			struct Desc
			{
				Desc() : bResettable(false) {};
				bool bResettable;
			};
			CommandFactory(Desc oDesc);
			~CommandFactory();

			VkCommandBuffer BeginSingleTimeCommands();
			void EndSingleTimeCommands(VkCommandBuffer oCommandBuffer);
			void FreeSingleTimeCommand(VkCommandBuffer oCommandBuffer);

			struct SubDrawDesc
			{
				std::shared_ptr< Buffer> xVertexData;
				std::shared_ptr< Buffer > xIndexData;
				Pipeline* pPipeline;
				VkDescriptorSet oDescriptorSet;
				int iInstanceCount;
			};
			const VkCommandPool* GetCommandPool() { return &m_oCommandPool; }

		protected:
			VkCommandPool m_oCommandPool;
		};
	}
}

#endif // !H_COMMAND_FACTORY
