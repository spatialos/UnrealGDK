// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{

ViewCoordinator::ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler)
: ConnectionHandler(MoveTemp(ConnectionHandler))
{
	Delta = View.GenerateViewDelta();
}

void ViewCoordinator::Advance()
{
	ConnectionHandler->Advance();
	const uint32 OpListCount = ConnectionHandler->GetOpListCount();
	for (uint32 i = 0; i < OpListCount; ++i)
	{
		View.EnqueueOpList(ConnectionHandler->GetNextOpList());
	}
	Delta = View.GenerateViewDelta();
}

void ViewCoordinator::FlushMessagesToSend()
{
	ConnectionHandler->SendMessages(View.FlushLocalChanges());
}

const TArray<CreateEntityResponse>& ViewCoordinator::GetCreateEntityResponses() const
{
	return Delta->GetCreateEntityResponses();
}

TUniquePtr<AbstractOpList> ViewCoordinator::GenerateLegacyOpList() const
{
	return Delta->GenerateLegacyOpList();
}

}  // SpatialView
