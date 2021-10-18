#include "SceneRayCastResults.h"

#include "Scene.h"

SceneRayCastResults::operator std::string() const
{
	if (Hit == nullptr)
		return "Nothing";

	std::stringstream out;

	out << Hit->Name << "{ Type: " << Hit->GetTypeName() << "; Distance: " << Distance << "; At: " << Intersection << " }";

	return out.str();
}