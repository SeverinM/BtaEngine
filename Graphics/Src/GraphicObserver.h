#ifndef H_GRAPHIC_OBSERVER
#define H_GRAPHIC_OBSERVER

namespace Bta
{
	namespace Graphics
	{
		enum E_GRAPHIC_EVENT
		{
			E_TRANSFORM_UPDATE,
			E_WINDOW_RESIZE
		};
	}
}

class Subject
{
	protected:
		Observer* m_pObserver;
		Subject(Observer* pObserver = nullptr) { m_pObserver = pObserver; }
		inline void SafeDispatch(E_GRAPHIC_EVENT eEvent) { if (m_pObserver != nullptr) m_pObserver->OnEvent(eEvent, this, nullptr); }
};

class Observer
{
public:
	virtual void OnEvent(E_GRAPHIC_EVENT eEvent, Subject* pAuthor, void* pData) = 0;
};

#endif