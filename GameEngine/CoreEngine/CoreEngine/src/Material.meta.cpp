#include "Material.h"

namespace GraphicsEngine
{
	using Engine::Object;

	Reflect_Inherited(Material, Object,
		Document_Class("");
		
		Document("");
		Archivable Class_Member(RGBA, Diffuse);

		Document("");
		Archivable Class_Member(RGBA, Specular);

		Document("");
		Archivable Class_Member(RGBA, Ambient);

		Document("");
		Archivable Class_Member(RGBA, Emission);

		Document("");
		Archivable Class_Member(int, Shininess);

		Document("");
		Archivable Class_Member(bool, BlendTexture);

		Document("");
		Archivable Class_Member(bool, CubeMapped);

		Document("");
		Archivable Class_Member(bool, FlipCubeMapV);

		Document("");
		Archivable Class_Member(bool, CompressedNormalMap);

		Document("");
		Archivable Class_Member(bool, UseOffsetMap);

		Document("");
		Archivable Class_Member(RGBA, TextureColor);

		Document("");
		Archivable Class_Member(Vector3, UVScale);

		Document("");
		Archivable Class_Member(Vector3, UVOffset);

		Document("");
		Archivable Class_Member(Vector3, BoxScale);

		Document("");
		Archivable Class_Member(std::weak_ptr<Texture>, DiffuseTexture);

		Document("");
		Archivable Class_Member(std::weak_ptr<Texture>, NormalMap);

		Document("");
		Archivable Class_Member(std::weak_ptr<Texture>, SpecularMap);
	);
}
