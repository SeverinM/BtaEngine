#ifndef H_VECTOR_UTILS
#define H_VECTOR_UTILS
#include <vector>

namespace Bta
{
	namespace Utils
	{
		class VectorUtils
		{
		public:
			template<typename T>
			static void InsertOrResize(std::vector<T>& oValue, int iIndex, T& oValueToInsert)
			{
				if (oValue.size() <= iIndex)
				{
					oValue.resize(iIndex + 1);
				}
				oValue[iIndex] = oValueToInsert;
			}

			template<typename T>
			static T& GetLastItem(std::vector<T>& oList)
			{
				return oList[oList.size() - 1];
			}
		};
	}
}

#endif
