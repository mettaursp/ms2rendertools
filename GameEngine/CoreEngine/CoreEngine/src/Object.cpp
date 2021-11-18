#include "Object.h"

#include <iostream>

#include "LuaBinding.h"

namespace Engine
{
	unsigned long long Object::ObjectsCreated = 0;
	Object::FactoryCallbackMap Object::FactoryFunctions;
	Object::ObjectHandleHeap Object::ObjectIDs = ObjectHandleHeap();

	void Object::Initialize()
	{
		ObjectID = ObjectIDs.RequestID(ObjectHandleData{ this, This, ++ObjectsCreated });
		OriginalID = ObjectID;
		CreationOrderId = ObjectsCreated;
	}

	bool Object::IsAlive(int objectId, unsigned long long creationOrderId)
	{
		return ObjectIDs.NodeAllocated(objectId) && ObjectIDs.GetNode(objectId).GetData().CreationOrderId == creationOrderId;
	}

	int Object::GetTypeID() const
	{
		return GetMetaData()->ID;
	}

	const std::string& Object::GetTypeName() const
	{
		return GetMetaData()->Name;
	}

	bool Object::IsA(const std::string& className, bool inherited) const
	{
		MetaData* data = GetMetaData();
		
		return data->Name == className || (inherited && data->Inherits(className));
	}

	bool Object::IsA(MetaData* metadata, bool inherited) const
	{
		MetaData* data = GetMetaData();

		return data == metadata || data->Inherits(metadata->Name);
	}

	const ClassData::Property* Object::GetProperty(const std::string& name) const
	{
		return reinterpret_cast<const ClassData*>(GetMetaData())->GetProperty(name);
	}

	bool Object::HasRequirements() const
	{
		return GetRequirements().size() > 0;
	}

	void Object::SetObjectID(int id)
	{
		if (ObjectID != -1)
			throw "Attempt to set ID";

		ObjectID = id;

		if (id != -1)
			OriginalID = id;
	}

	int Object::GetObjectID() const
	{
		return ObjectID;
	}

	Object::operator std::string() const
	{
		return Name;
	}

	const Object::FactoryCallback& Object::GetFactoryFunction(const std::string& className)
	{
		FactoryCallbackMap::iterator i = FactoryFunctions.find(className);

		if (i != FactoryFunctions.end())
			return i->second;

		throw "Attempt to get factory function of unregistered type: " + className;
	}

	std::shared_ptr<Object> CreateObject(const std::string& className)
	{
		return Object::GetFactoryFunction(className)();
	}

	void Object::Update(float delta)
	{
		for (int i = 0; i < GetChildren(); ++i)
		{
			if (Children[i] != nullptr && Children[i]->DoesTick())
				Children[i]->UpdateBase(delta);
			else if (Children[i] == nullptr)
				throw "bad child detected";
		}
	}

	Object::~Object()
	{
		auto parent = Parent.lock();

		if (parent != nullptr)
			parent->RemoveChild(This.lock());

		while (GetChildren() > 0)
		{
			if (Children.back() != nullptr)
				Children.back()->Parent.reset();

			Children.pop_back();
		}

		ObjectIDs.Release(ObjectID);
	}

	std::string Object::GetFullName() const
	{
		if (Parent.expired())
			return Name;

		std::string name = Name;
		
		for (std::shared_ptr<Object> parent = Parent.lock(); parent != nullptr && !parent->Parent.expired(); parent = parent->Parent.lock())
			name = parent->Name + "." + name;

		return name;
	}

	int Object::GetChildren() const
	{
		return int(Children.size());
	}

	std::shared_ptr<Object> Object::Get(const std::string& className, bool inherited)
	{
		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->IsA(className, inherited))
				return Children[i];

		return nullptr;
	}

	std::shared_ptr<Object> Object::Get(int index)
	{
		if (index < 0 || index >= int(Children.size()))
			return nullptr;

		return Children[index];
	}

	std::shared_ptr<Object> Object::GetByName(const std::string& name)
	{
		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->Name == name)
				return Children[i];

		return nullptr;
	}

	std::shared_ptr<Object> Object::GetAncestor(const std::string& className, bool inherited)
	{
		auto ancestor = Parent.lock();

		while (ancestor != nullptr)
		{
			if (ancestor->IsA(className, inherited))
				return ancestor;

			ancestor = ancestor->Parent.lock();
		}

		return nullptr;
	}

	std::shared_ptr<Object> Object::GetComponent(const std::string& className, bool inherited) const
	{
		return GetComponent(ReflectionData::GetType(className), inherited);
	}

	std::shared_ptr<Object> Object::GetComponent(MetaData* data, bool inherited) const
	{
		auto parent = Parent.lock();

		if (parent != nullptr)
		{
			if (ParentComponent && parent->IsA(data, inherited))
				return parent;

			if (SiblingComponents)
			{
				for (int i = 0; i < parent->GetChildren(); ++i)
				{
					std::shared_ptr<Object> child = parent->Cast<Object>()->Get(i)->Cast<Object>();

					if (child != This.lock() && child->IsA(data, inherited))
						return parent->Get(i);
				}
			}

			if (AncestorComponents || SuperSiblingComponents)
			{
				std::shared_ptr<Object> ancestor = Parent.lock();

				for (int height = 0; (SuperComponentHeight == -1 || height < SuperComponentHeight) && ancestor != nullptr; ++height)
				{
					if (AncestorComponents && ancestor->IsA(data, inherited))
						return ancestor;

					if (SuperSiblingComponents)
					{
						for (int i = 0; i < ancestor->GetChildren(); ++i)
						{
							std::shared_ptr<Object> child = ancestor->Get(i);

							if (child != This.lock() && child->IsA(data, inherited))
								return ancestor->Get(i);
						}
					}

					if (!ancestor->Parent.expired())
						ancestor = ancestor->Parent.lock();
					else
						ancestor = nullptr;
				}
			}
		}

		return nullptr;
	}

	bool Object::HasA(const std::string& className, bool inherited) const
	{
		if (IsA(className))
			return true;

		for (int i = 0; i < int(Children.size()); ++i)
			if (Children[i]->IsA(className, inherited))
				return true;

		return false;
	}

	void Object::AddChild(const std::shared_ptr<Object>& child)
	{
		if (!child->CheckRestriction(This.lock()))
			throw "Attempt to add duplicate child type: " + child->GetTypeName() + " restricts types of siblings; Adding \"" + child->Name + "\" to \"" + Name + "\"";

		const std::string& missingRequirement = child->CheckRequirements(This.lock());

		if (missingRequirement != "")
			throw "Attempt to add child to object with missing required siblings: " + child->GetTypeName() + " requires a sibling of type " + missingRequirement + "; Adding \"" + child->Name + "\" to \"" + Name + "\"";

		child->Parent = This;

		if (child->DoesTick())
		{
			++TickingChildren;

			UpdateTickingState();
		}

		Children.push_back(child);
	}

	void Object::Remove()
	{
		SetParent(nullptr);
	}

	void Object::RemoveChild(const std::shared_ptr<Object>& child)
	{
		if (child->DoesTick())
		{
			--TickingChildren;

			UpdateTickingState();
		}

		int index = 0;

		while (index < int(Children.size()) && Children[index] != child)
			++index;

		if (index < int(Children.size()))
		{
			if (index < int(Children.size()) - 1)
				Children[index] = Children[Children.size() - 1];

			Children.pop_back();
		}
	}

	void Object::SetParent(const std::shared_ptr<Object>& newParent)
	{
		if (newParent != nullptr && IsAncestorOf(newParent))
			throw "Attempt to create circular reference: " + GetTypeName() + " '" + Name + "' is already an ancestor of " + newParent->GetTypeName() + " '" + newParent->Name + "'";

		auto parent = Parent.lock();

		if (parent != nullptr)
			parent->RemoveChild(This.lock());

		if (newParent != nullptr)
			newParent->AddChild(This.lock());
		else
			Parent.reset();

		ParentChanged(newParent);
	}

	std::shared_ptr<Object> Object::GetParent() const
	{
		return Parent.lock();
	}

	bool Object::CheckRestriction(const std::shared_ptr<Object>& object) const
	{
		return !RestrictsSiblings() || object->HasA(GetTypeName());
	}

	std::string NoRequirements = "";

	const std::string& Object::CheckRequirements(const std::shared_ptr<Object>& object) const
	{
		if (!HasRequirements())
			return NoRequirements;

		const StringVector& requirements = GetRequirements();

		for (int i = 0; i < int(requirements.size()); ++i)
			if (!object->HasA(requirements[i]))
				return requirements[i];

		return NoRequirements;
	}

	bool Object::IsAncestorOf(const std::shared_ptr<Object>& object) const
	{
		if (object == nullptr)
			return false;

		return object->IsDescendantOf(This.lock());
	}

	bool Object::IsDescendantOf(const std::shared_ptr<Object>& object) const
	{
		if (object == nullptr)
			return false;

		auto focus = This.lock();

		while (focus != nullptr)
		{
			if (focus == object)
				return true;

			focus = focus->Parent.lock();
		}

		return false;
	}

	void Object::SetTicks(bool ticks)
	{
		Ticks = ticks;

		UpdateTickingState();
	}

	bool Object::DoesObjectTick() const
	{
		return Ticks;
	}

	bool Object::DoesTick() const
	{
		return Ticks || TickingChildren > 0;
	}

	void Object::UpdateTickingState()
	{
		bool ticksNow = DoesTick();

		std::shared_ptr<Object> parent = Parent.lock();

		if (parent != nullptr && TickedBefore != ticksNow)
		{
			if (ticksNow)
				++parent->TickingChildren;
			else
				--parent->TickingChildren;

			parent->UpdateTickingState();
		}

		TickedBefore = ticksNow;
	}

	namespace LuaTypes
	{
		int Lua_object::GetObjectID(const void* data)
		{
			LuaData* object = (LuaData*)(data);

			return object->Reference->GetObjectID();
		}

		void Lua_object::PushObject(lua_State* lua, const std::shared_ptr<Object>& object)
		{
			lua_pushstring(lua, "Objects");
			lua_gettable(lua, LUA_REGISTRYINDEX);

			int tableIndex = lua_gettop(lua);

			lua_pushnumber(lua, lua_Number(object->GetObjectID()));
			lua_gettable(lua, tableIndex);

			if (!lua_isuserdata(lua, lua_gettop(lua)))
			{
				int remove = 1;

				if (lua_gettop(lua) != tableIndex)
					remove = 2;

				lua_pop(lua, remove);

				MakeLuaReference(lua, object->GetObjectID());
			}
			else
				lua_replace(lua, -2);
		}

		void Lua_object::PushObject(lua_State* lua, const Reference<Engine::Object>& object)
		{
			PushObject(lua, object->Cast<Engine::Object>());
		}
	}
}
