#pragma once

#include <memory>

#include "IdentifierHeap.h"
#include "ObjectReflection.h"
#include "LuaStackTypes.h"
#include "ObjectAllocator.h"
#include "ObjectLuaType.h"
#include <iostream>

namespace Engine
{
	class Object
	{
	public:
		typedef std::function<std::shared_ptr<Object>()> FactoryCallback;
		typedef std::vector<std::string> StringVector;

		std::string Name = "Object";
		std::weak_ptr<Object> This;

		virtual ~Object();

		bool SiblingComponents = false;
		bool ParentComponent = true;
		bool AncestorComponents = true;
		bool SuperSiblingComponents = false;
		int SuperComponentHeight = -1;

		void Initialize();
		void Update(float delta);

		virtual void ParentChanged(std::shared_ptr<Object> newParent) {}

		int GetTypeID() const;
		const std::string& GetTypeName() const;
		bool IsA(const std::string& className, bool inherited = true) const;
		bool IsA(MetaData* type, bool inherited = true) const;
		const ClassData::Property* GetProperty(const std::string& name) const;

		bool HasRequirements() const;

		void SetObjectID(int id);
		int GetObjectID() const;

		template <typename T>
		bool IsA(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> Cast();

		template <typename T>
		std::shared_ptr<T> Cast() const;

		operator std::string() const;

		static const FactoryCallback& GetFactoryFunction(const std::string& className);

		template <typename T = Object>
		static std::shared_ptr<T> GetObjectFromID(int id);

		std::string GetFullName() const;
		int GetChildren() const;
		std::shared_ptr<Object> Get(const std::string& className, bool inherited = true);
		std::shared_ptr<Object> Get(int index);
		std::shared_ptr<Object> GetByName(const std::string& name);
		std::shared_ptr<Object> GetAncestor(const std::string& className, bool inherited = true);
		bool HasA(const std::string& className, bool inherited = true) const;

		void AddChild(const std::shared_ptr<Object>& child);
		void RemoveChild(const std::shared_ptr<Object>& child);
		void Remove();
		void SetParent(const std::shared_ptr<Object>& newParent);
		std::shared_ptr<Object> GetParent() const;
		std::shared_ptr<Object> GetComponent(const std::string& className, bool inherited = true) const;
		bool CheckRestriction(const std::shared_ptr<Object>& object) const;
		const std::string& CheckRequirements(const std::shared_ptr<Object>& object) const;
		bool IsAncestorOf(const std::shared_ptr<Object>& object) const;
		bool IsDescendantOf(const std::shared_ptr<Object>& object) const;

		void SetTicks(bool ticks);
		bool DoesTick() const;
		bool DoesObjectTick() const;

		template <typename T>
		std::shared_ptr<T> Get(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> Get(int index);

		template <typename T>
		std::shared_ptr<T> GetByName(const std::string& name);

		template <typename T>
		std::shared_ptr<T> GetAncestor(bool inherited = true);

		template <typename T>
		std::shared_ptr<T> GetComponent(bool inherited = true) const;

		template <typename T>
		bool HasA(bool inherited = true);

		const unsigned long long& GetCreationOrderId() const { return CreationOrderId; }

		static bool IsAlive(int objectID, unsigned long long creationOrderId);

	protected:
		static void RegisterFactoryFunction(const std::string& typeName, const FactoryCallback& factory)
		{
			FactoryFunctions[typeName] = factory;
		}

	private:
		struct ObjectHandleData
		{
			Object* Data = nullptr;
			std::weak_ptr<Object> SmartPointer;
			unsigned long long CreationOrderId = 0;
		};

		typedef std::map<std::string, FactoryCallback> FactoryCallbackMap;
		typedef IDHeap<ObjectHandleData> ObjectHandleHeap;
		typedef std::vector<std::shared_ptr<Object>> ObjectVector;

		static unsigned long long ObjectsCreated;
		static ObjectHandleHeap ObjectIDs;
		static FactoryCallbackMap FactoryFunctions;

		int ObjectID = -1;
		int OriginalID = -1;
		unsigned long long CreationOrderId = 0;
		bool Ticks = false;
		int TickingChildren = 0;
		bool TickedBefore = false;

		std::weak_ptr<Object> Parent;
		ObjectVector Children;

		std::shared_ptr<Object> GetComponent(MetaData* data, bool inherited) const;
		void UpdateTickingState();

		Instantiable;

		Base_Class;

		Reflected(Object);
	};

	template <typename T>
	std::shared_ptr<T> Object::Cast()
	{
		return std::dynamic_pointer_cast<T>(This.lock());
	}

	template <typename T>
	std::shared_ptr<T> Object::Cast() const
	{
		return std::dynamic_pointer_cast<T>(This.lock());
	}


	template <typename T>
	std::shared_ptr<T> Create()
	{
		return T::ObjectData::Create()->Cast<T>();
	}

	std::shared_ptr<Object> CreateObject(const std::string& className);

	template <typename T>
	std::shared_ptr<T> Object::Get(bool inherited)
	{
		for (int i = 0; i < GetChildren(); ++i)
			if (Get(i)->GetMetaData() == T::GetClassMetaData() || (inherited && Get(i)->GetMetaData()->Inherits(T::GetClassMetaData()->Name)))
				return Get(i)->Cast<T>();

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Object::Get(int index)
	{
		return Get(index)->Cast<T>();
	}

	template <typename T>
	std::shared_ptr<T> Object::GetByName(const std::string& name)
	{
		return GetByName(name)->Cast<T>();
	}

	template <typename T>
	std::shared_ptr<T> Object::GetAncestor(bool inherited)
	{
		std::shared_ptr<Object> ancestor = Parent.lock();

		while (ancestor != nullptr)
		{
			if (ancestor->GetMetaData() == T::GetClassMetaData() || (inherited && ancestor->GetMetaData()->Inherits(T::GetClassMetaData()->Name)))
				return ancestor;

			ancestor = ancestor->Parent;
		}

		return nullptr;
	}

	template <typename T>
	std::shared_ptr<T> Object::GetComponent(bool inherited) const
	{
		std::shared_ptr<Object> child = GetComponent(T::GetClassMetaData(), inherited);

		if (child == nullptr)
			return nullptr;

		return child->Cast<T>();
	}

	template <typename T>
	bool Object::HasA(bool inherited)
	{
		return HasA(T::GetClassMetaData()->Name, inherited);
	}

	template <typename T>
	bool Object::IsA(bool inherited)
	{
		return IsA(T::GetClassMetaData()->Name, inherited);
	}

	template <typename T>
	std::shared_ptr<T> Object::GetObjectFromID(int id)
	{
		if (id != -1)
			return ObjectIDs.GetNode(id).GetData().SmartPointer.lock()->Cast<T>();

		return nullptr;
	}
}
