#ifndef H_FILE_UTILS
#define  H_FILE_UTILS

#include <string>
#include <windows.h>
#include <iostream>

namespace Bta
{
	namespace Utils
	{
		class FileUtils
		{
			public:
				static std::wstring GetCurrentWorkDirectory()
				{
					TCHAR buffer[MAX_PATH] = { 0 };
					GetModuleFileName(NULL, buffer, MAX_PATH);
					std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
					return std::wstring(buffer).substr(0, pos);
				}
		};
	}
}

#endif
