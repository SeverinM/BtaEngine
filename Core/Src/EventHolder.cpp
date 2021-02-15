#include "EventHolder.h"

namespace Bta
{
	namespace Core
	{
		EventHolder* EventHolder::s_pHolder(nullptr);

		EventHolder* EventHolder::GetInstance()
		{
			if (s_pHolder == nullptr)
			{
				s_pHolder = new EventHolder();
			}

			return s_pHolder;
		}

		void EventHolder::PushEvent(E_COMMAND_TYPE eCommandType, BaseEvent oBaseCommand)
		{
			if (m_oAllCommands[eCommandType].size() < 10)
			{
				m_oAllCommands[eCommandType].push(oBaseCommand);
			}
		}

		void EventHolder::ConsumeEvent(E_COMMAND_TYPE eCommandType)
		{
			m_oAllCommands[eCommandType].pop();
		}

	}
}