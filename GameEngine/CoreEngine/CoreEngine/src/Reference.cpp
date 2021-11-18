#include "Reference.h"

#include "Object.h"

bool IsObjectAlive(int objectId, unsigned long long creationOrderId)
{
	return Engine::Object::IsAlive(objectId, creationOrderId);
}

int GetObjectId(const Engine::Object* object)
{
	return object->GetObjectID();
}

const unsigned long long& GetObjectCreationId(const Engine::Object* object)
{
	return object->GetCreationOrderId();
}