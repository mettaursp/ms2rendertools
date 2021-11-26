#include "Material.h"

namespace GraphicsEngine
{
	IDHeap<Material*> Material::MaterialRegistry = IDHeap<Material*>();

	Material::~Material()
	{
		MaterialRegistry.Release(MaterialId);

		MaterialId = -1;
	}

	void Material::Initialize()
	{
		MaterialId = MaterialRegistry.RequestID(this);
	}

	Material* Material::GetMaterialFromId(int id)
	{
		if (!MaterialRegistry.NodeAllocated(id))
			return nullptr;

		return MaterialRegistry.GetNode(id).GetData();
	}
}
