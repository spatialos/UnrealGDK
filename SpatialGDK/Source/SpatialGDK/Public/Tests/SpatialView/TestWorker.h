// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ViewCoordinator.h"
#include "TestConnectionHandler.h"
#include "Tests/SpatialView/TargetView.h"

namespace SpatialGDK
{
class FTestWorker
{
public:
	static FTestWorker Create(TArray<Worker_ComponentSetId> ComponentSetIds, FString InWorkerId = {},
							  Worker_EntityId InWorkerSystemEntityId = 0)
	{
		TUniquePtr<FTestConnectionHandler> Handler = MakeUnique<FTestConnectionHandler>(InWorkerId, InWorkerSystemEntityId);
		FComponentSetData SetData;
		for (Worker_ComponentSetId SetId : ComponentSetIds)
		{
			SetData.ComponentSets.Add(SetId, {});
		}
		return FTestWorker(MoveTemp(Handler), MoveTemp(SetData));
	}

	void SetSendMessageCallback(FSendMessageCallback InSendMessageCallback)
	{
		ConnectionHandler->SetSendMessageCallback(MoveTemp(InSendMessageCallback));
	}

	FTargetView& GetTargetView() { return TargetView; }

	void AdvanceToTargetView(float DeltaTimeS)
	{
		ConnectionHandler->AddOpList(TargetView.CreateOpListFromChanges());
		Coordinator.Advance(DeltaTimeS);
	}

	ViewCoordinator& GetCoordinator() { return Coordinator; }
	const ViewCoordinator& GetCoordinator() const { return Coordinator; }

private:
	explicit FTestWorker(TUniquePtr<FTestConnectionHandler> Handler, FComponentSetData SetData)
		: ConnectionHandler(Handler.Get())
		, Coordinator(MoveTemp(Handler), /*EventTracer =*/nullptr, MoveTemp(SetData))
	{
	}

	FTestConnectionHandler* ConnectionHandler;
	FTargetView TargetView;
	ViewCoordinator Coordinator;
};

} // namespace SpatialGDK
