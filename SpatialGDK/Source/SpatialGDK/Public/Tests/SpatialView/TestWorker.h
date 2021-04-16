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
	explicit FTestWorker(TArray<Worker_ComponentSetId> ComponentSetIds, FString InWorkerId = {}, Worker_EntityId InWorkerSystemEntityId = 0)
	{
		TUniquePtr<FTestConnectionHandler> Handler = MakeUnique<FTestConnectionHandler>(InWorkerId, InWorkerSystemEntityId);
		ConnectionHandler = Handler.Get();
		FComponentSetData SetData;
		for (Worker_ComponentSetId SetId : ComponentSetIds)
		{
			SetData.ComponentSets.Add(SetId, {});
		}
		Coordinator = ViewCoordinator(MoveTemp(Handler), nullptr, MoveTemp(SetData));
	}

	void SetSendMessageCallback(FSendMessageCallback InSendMessageCallback)
	{
		ConnectionHandler->SetSendMessageCallback(MoveTemp(InSendMessageCallback));
	}

	void AdvanceToTargetView(float DeltaTimeS)
	{
		ConnectionHandler->AddOpList(TargetView.CreateOpListFromChanges());
		Coordinator.Advance(DeltaTimeS);
	}

	ViewCoordinator& GetCoordinator() { return Coordinator; }
	const ViewCoordinator& GetCoordinator() const { return Coordinator; }

private:
	FTestConnectionHandler* ConnectionHandler;
	FTargetView TargetView;
	ViewCoordinator Coordinator;
};

} // namespace SpatialGDK
