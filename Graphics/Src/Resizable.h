#ifndef H_RESIZABLE
#define H_RESIZABLE

class Resizable
{
public:
	virtual void Free() = 0;
	virtual void Recreate(int iNewWidth , int iNewHeight, void* pData) = 0;
};

#endif H_RESIZABLE
