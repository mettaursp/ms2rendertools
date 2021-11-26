#pragma once

#include <memory>

namespace Engine
{
	class Object;
}

bool IsObjectAlive(int objectId, unsigned long long creationOrderId);
int GetObjectId(const Engine::Object* object);
const unsigned long long& GetObjectCreationId(const Engine::Object* object);

template <typename T>
class Reference
{
public:
	Reference(T* data = nullptr) { *this = data; }
	Reference(const Reference& data) { *this = data; }
	Reference(const std::shared_ptr<T>& data) { *this = data; }
	Reference(const std::weak_ptr<T>& data) { *this = data; }

	Reference& operator=(T* data) { Set(data); return *this; }

	Reference& operator=(const Reference& data)
	{
		Data = data.Data;
		ObjectId = data.ObjectId;
		CreationOrderId = data.CreationOrderId;

		return *this;
	}

	Reference& operator=(const std::shared_ptr<T>& data) { Set(data.get()); return *this; }
	Reference& operator=(const std::weak_ptr<T>& data) { Set(data.lock().get()); return *this; }

	T* operator->() { return Get(); }
	T* operator->() const { return Get(); }
	T* operator*() { return Get(); }
	T* operator*() const { return Get(); }

	bool operator==(T* data) const { return Data == data; }
	bool operator==(const Reference& data) const { return Data == data.Data; }
	bool operator==(const std::shared_ptr<T>& data) const { return Data == data.get(); }
	bool operator==(const std::weak_ptr<T>& data) const { return Data == data.lock().get(); }

	operator T*() const { return Data; }
	//operator std::shared_ptr<T>() const { if (Data == nullptr) return nullptr; return Data->Cast<T>(); }
	//operator std::weak_ptr<T>() const { if (Data == nullptr) return nullptr; return Data->Cast<T>(); }

private:
	T* Data = nullptr;
	int ObjectId = -1;
	unsigned long long CreationOrderId = 0;

	void Set(T* data)
	{
		if (data == nullptr)
			ObjectId = -1;
		else
		{
			Data = data;
			ObjectId = GetObjectId(reinterpret_cast<Engine::Object*>(Data));
			CreationOrderId = GetObjectCreationId(reinterpret_cast<Engine::Object*>(Data));
		}
	}

	T* Get() const
	{
		if (Data == nullptr && IsObjectAlive(ObjectId, CreationOrderId))
			return Data;

		return nullptr;
	}

	T* Get()
	{
		if (Data == nullptr && IsObjectAlive(ObjectId, CreationOrderId))
			return Data;

		Data = nullptr;

		return nullptr;
	}
};