// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Engine/ActorChannel.h"
#include "EntityId.h"
#include "SpatialOSCommandResult.h"
#include "Commander.h"
#include "improbable/worker.h"
#include "improbable/standard_library.h"
#include <map>
#include "SpatialActorChannel.generated.h"


// A replacement actor channel that plugs into the Engine's replication system and works with SpatialOS
UCLASS(Transient)
class NUF_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & objectInitializer = FObjectInitializer::Get());

	// UChannel interface
	virtual void Init(UNetConnection * connection, int32 channelIndex, bool bOpenedLocally) override;
	virtual void SetClosingFlag() override;
	virtual void Close() override;
	virtual void ReceivedBunch(FInBunch & bunch) override;
	virtual void ReceivedNak(int32 packetId) override;
	virtual void Tick() override;
	virtual bool CanStopTicking() const override;
	virtual void AppendExportBunches(TArray<FOutBunch *> & outExportBunches) override;
	virtual void AppendMustBeMappedGuids(FOutBunch * bunch) override;
	virtual FPacketIdRange SendBunch(FOutBunch * bunch, bool bMerge) override;
	virtual void StartBecomingDormant() override;
	//NUF-sourcechange Requires ActorChannel.h
	virtual bool ReplicateActor() override;


	void OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op);
	void OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op);

protected:
	// UChannel interface
	virtual void BecomeDormant() override;
	virtual bool CleanUp(const bool bForDestroy) override;

private:
	
	TWeakPtr<worker::Connection> WorkerConnection;
	TWeakPtr<worker::View> WorkerView;	

	TUniquePtr<improbable::unreal::callbacks::FScopedViewCallbacks> Callbacks;

	worker::RequestId<worker::ReserveEntityIdRequest> ReserveEntityIdRequestId;	
	worker::RequestId<worker::CreateEntityRequest> CreateEntityRequestId;
};
