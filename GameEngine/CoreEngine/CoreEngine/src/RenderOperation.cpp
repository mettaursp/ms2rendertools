#include "RenderOperation.h"

namespace GraphicsEngine
{
	void RenderOperation::Initialize()
	{
		OperationID = GetOperations().RequestID(This.lock()->Cast<RenderOperation>());
	}

	RenderOperation::~RenderOperation()
	{
		if (OperationID != -1)
			GetOperations().Release(OperationID);
		
		OperationID = -1;
	}

	void RenderOperation::UpdateOperations(float delta)
	{
		OperationHeap& operations = GetOperations();

		for (int i = 0; i < operations.Size(); ++i)
		{
			if (operations.NodeAllocated(i))
			{
				std::shared_ptr<RenderOperation> operation = operations.GetNode(i).GetData().lock();

				if (operation == nullptr)
					operations.Release(i);
				else if (operation->RenderAutomatically)
					operation->Update(delta);
			}
		}
	}

	void RenderOperation::RenderOperations()
	{
		OperationHeap& operations = GetOperations();

		for (int i = 0; i < operations.Size(); ++i)
		{
			if (operations.NodeAllocated(i))
			{
				std::shared_ptr<RenderOperation> operation = operations.GetNode(i).GetData().lock();

				if (operation == nullptr)
					operations.Release(i);
				else// if (operation->RenderAutomatically)
					operation->UpdateBase(0, true);
			}
		}

		for (int i = 0; i < operations.Size(); ++i)
		{
			if (operations.NodeAllocated(i))
			{
				std::shared_ptr<RenderOperation> operation = operations.GetNode(i).GetData().lock();

				if (operation->RenderAutomatically)
					operation->Render();
			}
		}
	}

	RenderOperation::OperationHeap& RenderOperation::GetOperations()
	{
		static OperationHeap operations;

		return operations;
	}
}