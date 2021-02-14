#include "MeshComponent.h"

int main()
{
	Bta::Core::Entity oEntity(nullptr);

	uint8_t iSizes[] = { 3,3,2,4 };
	Bta::Core::MeshComponent* pMeshComponent = new Bta::Core::MeshComponent(iSizes);
	oEntity.AddExistingComponent(pMeshComponent);
	
	Bta::Core::MeshComponent* pTest = oEntity.FindFirstComponent<Bta::Core::MeshComponent>();
	Bta::Core::Vertice oVertice;
	oVertice.vColor = glm::vec4(0, 1, 0.4f, 0.2f);
	oVertice.vNormal = glm::vec3(1.4f, 0.2f, -9.3f);
	oVertice.vPosition = glm::vec3(0.1f, -0.2f, 0.1f);
	oVertice.vUV = glm::vec2(0.1f, 0.2f);
	pTest->AddVertice(oVertice);

	return 0;
}