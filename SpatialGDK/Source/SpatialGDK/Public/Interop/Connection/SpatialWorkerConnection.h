// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/Connection/OutgoingMessages.h"
#include "Interop/Connection/SpatialOSWorkerInterface.h"
#include "SpatialCommonTypes.h"

#include "SpatialWorkerConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkerConnection, Log, All);

UCLASS(abstract)
class SPATIALGDK_API USpatialWorkerConnection : public UObject, public SpatialOSWorkerInterface
{
	GENERATED_BODY()

public:
	virtual void SetConnection(Worker_Connection* WorkerConnectionIn) PURE_VIRTUAL(USpatialWorkerConnection::SetConnection, return;);
	virtual void FinishDestroy() override { Super::FinishDestroy(); }
	virtual void DestroyConnection() PURE_VIRTUAL(USpatialWorkerConnection::DestroyConnection, return;);

	virtual PhysicalWorkerName GetWorkerId() const PURE_VIRTUAL(USpatialWorkerConnection::GetWorkerId, return PhysicalWorkerName(););
	virtual const TArray<FString>& GetWorkerAttributes() const
		PURE_VIRTUAL(USpatialWorkerConnection::GetWorkerAttributes, return ReturnValuePlaceholder;);

	virtual void ProcessOutgoingMessages() PURE_VIRTUAL(USpatialWorkerConnection::ProcessOutgoingMessages, return;);
	virtual void MaybeFlush() PURE_VIRTUAL(USpatialWorkerConnection::MaybeFlush, return;);
	virtual void Flush() PURE_VIRTUAL(USpatialWorkerConnection::Flush, return;);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnqueueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnEnqueueMessage OnEnqueueMessage;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDequeueMessage, const SpatialGDK::FOutgoingMessage*);
	FOnDequeueMessage OnDequeueMessage;

private:
	// Exists for the sake of having PURE_VIRTUAL functions returning a const ref.
	TArray<FString> ReturnValuePlaceholder;
};
