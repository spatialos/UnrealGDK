// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "Engine/ActorChannel.h"
#include "SpatialActorChannel.generated.h"

// A replacement actor channel that plugs into the Engine's replication system and works with SpatialOS
UCLASS(Transient)
class NUF_API USpatialActorChannel : public UActorChannel
{
	GENERATED_BODY()

public:
	USpatialActorChannel(const FObjectInitializer & ObjectInitializer = FObjectInitializer::Get());

	// SpatialOS Entity ID.
	worker::EntityId GetEntityId() const;

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

protected:
	// UChannel interface
	virtual void BecomeDormant() override;
	virtual bool CleanUp(const bool bForDestroy) override;

};
