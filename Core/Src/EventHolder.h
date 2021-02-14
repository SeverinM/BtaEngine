#ifndef H_COMMAND_HOLDER
#define H_COMMAND_HOLDER

#include <stack>
#include <unordered_map>

namespace Bta
{
	namespace Core
	{
		enum E_COMMAND_TYPE
		{
			E_MODIFIED_VERTICE_BUFFER = 0,
			E_MODIFIED_INDEX_BUFFER,
			E_NEW_COMPONENT
		};

		struct BaseEvent
		{
			BaseEvent() : pSubject(nullptr) {};
			void* pSubject;
		};

		class EventHolder
		{
			private:
				static EventHolder* s_pHolder;
				static const int s_iStackLimit;
				std::unordered_map<int, std::stack<BaseEvent>> m_oAllCommands;
				EventHolder() {};

			public:
				static EventHolder* GetInstance();
				void PushEvent(E_COMMAND_TYPE eCommandType, BaseEvent oBaseCommand);
				void ConsumeEvent(E_COMMAND_TYPE eCommandType);
				BaseEvent PeekEvent(E_COMMAND_TYPE eCommandType) { return m_oAllCommands[eCommandType].top(); }
		};
	}
}

#endif
