// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IpConnection.h"

<<<<<<< HEAD
#include "Schema/Interest.h"

#include "SpatialNetConnection.generated.h"

using namespace improbable;

struct ClientInterest
{
	TSet<uint32> LoadedLevels;

	inline improbable::ComponentInterest CreateComponentInterest()
	{
		ComponentInterest::QueryConstraint CheckoutRadiusConstraint;
		CheckoutRadiusConstraint.RelativeCylinderConstraint = ComponentInterest::RelativeCylinderConstraint{ 50 };

		ComponentInterest::QueryConstraint DefaultCheckout;
		DefaultCheckout.ComponentConstraint = SpatialConstants::NOT_SPAWNED_COMPONENT_ID;

		ComponentInterest::QueryConstraint DefaultConstraint;
		DefaultConstraint.AndConstraint.Add(CheckoutRadiusConstraint);
		DefaultConstraint.AndConstraint.Add(DefaultCheckout);

		ComponentInterest::Query FullInterest;
		FullInterest.Constraint.OrConstraint.Add(DefaultConstraint);

		for (const auto& LevelComponentId : LoadedLevels)
		{
			ComponentInterest::QueryConstraint LevelConstraint;
			LevelConstraint.ComponentConstraint = LevelComponentId;

			ComponentInterest::QueryConstraint FullLevelConstraint;
			FullLevelConstraint.AndConstraint.Add(CheckoutRadiusConstraint);
			FullLevelConstraint.AndConstraint.Add(LevelConstraint);

			FullInterest.Constraint.OrConstraint.Add(FullLevelConstraint);
		}

		FullInterest.FullSnapshotResult = true;

		ComponentInterest CurrentInterest;
		CurrentInterest.Queries.Add(FullInterest);

		return CurrentInterest;
	}
};
=======
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialNetConnection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialNetConnection, Log, All);
>>>>>>> 9aa87f97101184bab1766ded34b5a4e735eac321

UCLASS(transient)
class SPATIALGDK_API USpatialNetConnection : public UIpConnection
{
	GENERATED_BODY()
public:
	USpatialNetConnection(const FObjectInitializer& ObjectInitializer);

	virtual void BeginDestroy() override;

	virtual void InitBase(UNetDriver* InDriver, class FSocket* InSocket, const FURL& InURL, EConnectionState InState, int32 InMaxPacket = 0, int32 InPacketOverhead = 0) override;
	virtual void LowLevelSend(void* Data, int32 CountBytes, int32 CountBits) override;
	virtual bool ClientHasInitializedLevelFor(const AActor* TestActor) const override;
	virtual void Tick() override;
	virtual int32 IsNetReady(bool Saturate) override;

	/** Called by PlayerController to tell connection about client level visiblity change */
	virtual void UpdateLevelVisibility(const FName& PackageName, bool bIsVisible) override;

	// These functions don't make a lot of sense in a SpatialOS implementation.
	virtual FString LowLevelGetRemoteAddress(bool bAppendPort = false) override { return TEXT(""); }
	virtual FString LowLevelDescribe() override { return TEXT(""); }
	virtual FString RemoteAddressToString() override { return TEXT(""); }
	///////

	void InitHeartbeat(class FTimerManager* InTimerManager, Worker_EntityId InPlayerControllerEntity);
	void SetHeartbeatTimeoutTimer();
	void SetHeartbeatEventTimer();

	void DisableHeartbeat();

	void OnHeartbeat();

	UPROPERTY()
	bool bReliableSpatialConnection;

	UPROPERTY()
	FString WorkerAttribute;

<<<<<<< HEAD
	ClientInterest CurrentInterest;
=======
	class FTimerManager* TimerManager;

	// Player lifecycle
	Worker_EntityId PlayerControllerEntity;
	FTimerHandle HeartbeatTimer;
>>>>>>> 9aa87f97101184bab1766ded34b5a4e735eac321
};
