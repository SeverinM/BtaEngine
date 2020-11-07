#ifndef H_DELAYED_COMMANDS
#define H_DELAYED_COMMANDS
#include <functional>
#include <unordered_map>
#include <iostream>

class DelayedCommands
{

public:
	struct QueueCommands
	{
		std::function<void()> oOnStart;
		std::function<void()> oTimeOutFunction;
	};

	void PushCommand(QueueCommands oCommand, float fTime)
	{
		oCommand.oOnStart();

		m_oFunctions.push_back(oCommand);
		m_oTimes.push_back(fTime);
	}

	void Update(float fElapsed)
	{
		if (m_oTimes.size() == 0)
			return;

		for (int i = m_oTimes.size() - 1; i >= 0; i--)
		{
			m_oTimes[i] -= fElapsed;
			if (m_oTimes[i] < 0)
			{
				m_oFunctions[i].oTimeOutFunction();
				m_oFunctions.erase(m_oFunctions.begin() + i);
				m_oTimes.erase(m_oTimes.begin() + i);
			}
		}
	}
	

private:
	int m_iMaxSize;
	std::vector<float> m_oTimes;
	std::vector<QueueCommands> m_oFunctions;
};

#endif