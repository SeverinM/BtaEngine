#ifndef H_GRAPHIC_UTILS
#define H_GRAPHIC_UTILS

#include "./MeshComponent.h"
#include <vector>

namespace Bta
{
	namespace Graphic
	{
		class GraphicUtils
		{
			public:

				struct OutputMesh
				{
					std::vector<Vertice> vertices;
					std::vector<MeshComponent::IndexType> indices;
				};

				static OutputMesh CreateBox()
				{
					OutputMesh oOutMesh;
					oOutMesh.vertices.resize(8);

					Vertice vert;
					vert.vUV = glm::vec2(0, 0);
					vert.vColor = glm::vec4(1, 1, 1, 1);
					vert.vNormal = glm::vec3(0, 1, 0);
					vert.vPosition = glm::vec3(-0.5, -0.5, -0.5f);
					oOutMesh.vertices[0] = vert;
					vert.vPosition = glm::vec3(0.5f, -0.5f, 0.5f);
					oOutMesh.vertices[1] = vert;
					vert.vPosition = glm::vec3(0.5f, -0.5f, -0.5f);
					oOutMesh.vertices[2] = vert;
					vert.vPosition = glm::vec3(-0.5f,-0.5f, 0.5f);
					oOutMesh.vertices[3] = vert;
					vert.vPosition = glm::vec3(-0.5f, 0.5f, -0.5f);
					oOutMesh.vertices[4] = vert;
					vert.vPosition = glm::vec3(0.5f, 0.5f, 0.5f);
					oOutMesh.vertices[5] = vert;
					vert.vPosition = glm::vec3(0.5f, 0.5f, -0.5f);
					oOutMesh.vertices[6] = vert;
					vert.vPosition = glm::vec3(-0.5f, 0.5f, 0.5f);
					oOutMesh.vertices[7] = vert;

					oOutMesh.indices.resize(36);

					//Bottom
					oOutMesh.indices[0] = 0;
					oOutMesh.indices[1] = 2;
					oOutMesh.indices[2] = 1;
					oOutMesh.indices[3] = 0;
					oOutMesh.indices[4] = 1;
					oOutMesh.indices[5] = 3;

					//Top
					oOutMesh.indices[6] = 4;
					oOutMesh.indices[7] = 6;
					oOutMesh.indices[8] = 5;
					oOutMesh.indices[9] = 4;
					oOutMesh.indices[10] = 5;
					oOutMesh.indices[11] = 7;

					//Back
					oOutMesh.indices[12] = 3;
					oOutMesh.indices[13] = 1;
					oOutMesh.indices[14] = 5;
					oOutMesh.indices[15] = 3;
					oOutMesh.indices[16] = 5;
					oOutMesh.indices[17] = 7;

					//Front
					oOutMesh.indices[18] = 0;
					oOutMesh.indices[19] = 2;
					oOutMesh.indices[20] = 6;
					oOutMesh.indices[21] = 0;
					oOutMesh.indices[22] = 4;
					oOutMesh.indices[23] = 6;

					//Left
					oOutMesh.indices[24] = 0;
					oOutMesh.indices[25] = 3;
					oOutMesh.indices[26] = 4;
					oOutMesh.indices[27] = 4;
					oOutMesh.indices[28] = 7;
					oOutMesh.indices[29] = 4;

					//Right
					oOutMesh.indices[30] = 2;
					oOutMesh.indices[31] = 1;
					oOutMesh.indices[32] = 6;
					oOutMesh.indices[33] = 1;
					oOutMesh.indices[34] = 6;
					oOutMesh.indices[35] = 5;

					return oOutMesh;
				}
		};
	}
}

#endif
