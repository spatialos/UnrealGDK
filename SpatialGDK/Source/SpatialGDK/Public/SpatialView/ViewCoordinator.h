// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "SpatialView/WorkerView.h"
#include "SpatialView/ConnectionHandlers/AbstractConnectionHandler.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{

class ViewCoordinator
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler);

	void Advance();
	void FlushMessagesToSend();

	const TArray<CreateEntityResponse>& GetCreateEntityResponses() const;
	const TArray<EntityComponentId>& GetAuthorityGained() const;
	const TArray<EntityComponentId>& GetAuthorityLost() const;
	const TArray<EntityComponentId>& GetAuthorityLostTemporarily() const;

	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

private:
	const ViewDelta* Delta;
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
};

}  // namespace SpatialGDK
