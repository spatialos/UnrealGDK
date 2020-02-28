// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "WorkerView.h"
#include "ConnectionHandlers/AbstractConnectionHandler.h"
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

	TUniquePtr<AbstractOpList> GenerateLegacyOpList() const;

private:
	const ViewDelta* Delta;
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
};

}  // namespace SpatialGDK
