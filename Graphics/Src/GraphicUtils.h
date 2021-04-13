#ifndef H_GRAPHIC_UTILS
#define H_GRAPHIC_UTILS

#include "./MeshComponent.h"

namespace Bta
{
	namespace Graphic
	{
		class GraphicUtils
		{
			public:

				struct OutputMesh
				{
					Vertice* vertices;
					MeshComponent::IndexType* indices;
				};

				static OutputMesh CreateBox()
				{
					OutputMesh oOutMesh;

					Vertice oOutput[8];
					Vertice vert;
					vert.vUV = glm::vec2(0, 0);
					vert.vColor = glm::vec4(1, 1, 1, 1);
					vert.vNormal = glm::vec3(0, 1, 0);
					vert.vPosition = glm::vec3(0, 0, 0);
					oOutput[0] = vert;
					vert.vPosition = glm::vec3(1, 0, 1);
					oOutput[1] = vert;
					vert.vPosition = glm::vec3(1, 0, 0);
					oOutput[2] = vert;
					vert.vPosition = glm::vec3(0, 0, 1);
					oOutput[3] = vert;
					vert.vPosition = glm::vec3(0, 1, 0);
					oOutput[4] = vert;
					vert.vPosition = glm::vec3(1, 1, 1);
					oOutput[5] = vert;
					vert.vPosition = glm::vec3(1, 1, 0);
					oOutput[6] = vert;
					vert.vPosition = glm::vec3(0, 1, 1);
					oOutput[7] = vert;
					
					oOutMesh.vertices = oOutput;

					return oOutMesh;
				}
		};
	}
}

#endif
