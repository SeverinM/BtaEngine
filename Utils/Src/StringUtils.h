#ifndef H_STRING_UTILS
#define H_STRING_UTILS
#include <string>
#include <iostream>
#include "GLM/glm.hpp"

namespace Bta
{
	namespace Utils
	{
		class StringUtils
		{
		public:

			static std::string FetchFirstDigits(std::string& sString)
			{
				bool bDigitFound = false;
				int iStart = -1;
				int iEnd = -1;

				for (int i = 0; i < sString.size(); i++)
				{
					if (bDigitFound && ( !isdigit(sString[i]) || i == sString.size() - 1) )
					{
						bDigitFound = false;
						iEnd = i;
						break;
					}

					if (!bDigitFound && isdigit(sString[i]))
					{
						bDigitFound = true;
						iStart = i;
					}
				}

				if (iStart == -1)
				{
					return "";
				}

				return sString.substr(iStart, iEnd - iStart);
			}

			static bool Contains(std::string& sString, std::string sPattern)
			{
				return sString.find(sPattern) != std::string::npos;
			}

			static bool StartWith(std::string& sString, std::string sPattern, bool bStripFirst = true)
			{
				if (bStripFirst)
					StripSpace(sString);

				return sString.substr(0, sPattern.size()) == sPattern;
			}

			static void StripSpace(std::string& sString)
			{
				RemoveStartSpace(sString);
				RemoveEndSpace(sString);
			}

			static void RemoveStartSpace(std::string& sString)
			{
				std::string::iterator pBegin = sString.begin();
				std::string::iterator pEnd = sString.begin();
				while ( (*pEnd) == ' ' && pEnd != sString.end())
				{
					pEnd++;
				}
				sString.erase(pBegin, pEnd);
			}

			static void RemoveEndSpace(std::string& sString)
			{
				std::string::iterator pBegin = sString.end() - 1;
				std::string::iterator pEnd = sString.end();

				while ((*pBegin) == ' ' && pBegin != sString.begin())
				{
					pBegin--;
				}
				sString.erase(pBegin + 1, pEnd);
			}

			static std::vector<std::string> Split(std::string& oSample, char pattern)
			{
				std::vector<std::string> oOutput;
				bool bCreatingSub = false;
				int iBegin = -1;
				int iEnd = -1;
				for (int i = 0; i < oSample.size(); i++)
				{
					if (!bCreatingSub && oSample[i] != pattern)
					{
						bCreatingSub = true;
						iBegin = i;
					}
					
					if (bCreatingSub && ( oSample[i] == pattern || i == oSample.size() - 1 ) )
					{
						bCreatingSub = false;
						iEnd = i;

						if (iEnd == oSample.size() - 1)
						{
							iEnd++;
						}

						oOutput.push_back(oSample.substr(iBegin, iEnd - iBegin));
					}
				}
				return oOutput;
			}

			static size_t ParseMemorySize(std::string& sText)
			{
				if (Bta::Utils::StringUtils::StartWith(sText, "mat4"))
				{
					return sizeof(glm::mat4);
				}
				else if (Bta::Utils::StringUtils::StartWith(sText, "vec3"))
				{
					return sizeof(glm::vec3);
				}
				else if (Bta::Utils::StringUtils::StartWith(sText, "vec2"))
				{
					return sizeof(glm::vec2);
				}
				else
				{
					return -1;
				}
			}
		};
	}
}

#endif
