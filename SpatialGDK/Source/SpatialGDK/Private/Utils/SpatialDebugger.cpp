// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialDebugger.h"

#include "Components/SceneComponent.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Engine.h"
#include "EngineClasses/SpatialLoadBalancingStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/SpatialStatics.h"
#include "Net/UnrealNetwork.h"
#include "Schema/StandardLibrary.h"

using namespace SpatialGDK;

ASpatialDebugger::ASpatialDebugger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxRange(100.0f * 100.0f)
	, bShowAuth(true)
	, bShowAuthIntent(true)
	, bShowLock(true)
	, bShowEntityId(true)
	, bShowActorName(true)
	, bStack(false)
	, bAutoStart(true)
	, LocalPawn(nullptr)
	, LocalPlayerController(nullptr)
	, LocalPlayerState(nullptr)
	, RenderFont(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 1.f;

	// For GDK design reasons, this is the approach chosen to get a pointer
	// on the net driver to the client ASpatialDebugger.  Various alternatives
	// were considered and this is the best of a bad bunch.
	if (USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver()))
	{
		if (GetNetDriver()->IsServer() == false)
		{
			SpatialNetDriver->SetSpatialDebugger(this);
		}
	}
}

void ASpatialDebugger::Tick(float DeltaSeconds)
{
	if (GetNetDriver() &&
		GetNetDriver()->IsServer() == false)
	{
		// Since we have no guarantee on the order we'll receive the PC/Pawn/PlayerState
		// over the wire, we check here once per tick (currently 1 Hz tick rate) to setup our local pointers.
		// Note that we can capture the PC in OnEntityAdded() since we know we will only receive one of those.
		if (LocalPlayerController && !LocalPawn)
		{
			LocalPawn = LocalPlayerController->GetPawn();
		}

		if (LocalPawn && !LocalPlayerState)
		{
			LocalPlayerState = LocalPawn->GetPlayerState();
		}
	}
}

void ASpatialDebugger::BeginPlay()
{
	Super::BeginPlay();

	check(GetNetDriver() != nullptr);

	if (!GetNetDriver()->IsServer())
	{
		EntityActorMapping.Reserve(ENTITY_ACTOR_MAP_RESERVATION_COUNT);
		ActorLocationCountMapping.Reserve(POSITION_HASH_BUCKET_RESERVATION_COUNT);

		LoadIcons();

		TArray<Worker_EntityId_Key> EntityIds;
		Cast<USpatialNetDriver>(GetNetDriver())->StaticComponentView->GetEntityIds(EntityIds);

		// Capture any entities that are already present on this client (ie they came over the wire before the SpatialDebugger did)
		for (const Worker_EntityId_Key EntityId : EntityIds)
		{
			OnEntityAdded(EntityId);
		}

		// Register callbacks to get notified of all future entity arrivals / deletes
		Cast<USpatialNetDriver>(GetNetDriver())->Receiver->OnEntityAdded.BindUObject(this, &ASpatialDebugger::OnEntityAdded);
		Cast<USpatialNetDriver>(GetNetDriver())->Receiver->OnEntityRemoved.BindUObject(this, &ASpatialDebugger::OnEntityRemoved);

		FontRenderInfo.bClipText = true;
		FontRenderInfo.bEnableShadow = true;

		RenderFont = GEngine->GetSmallFont();

		if (bAutoStart)
		{
			SpatialToggleDebugger();
		}
	}
}

void ASpatialDebugger::Destroyed()
{
	Super::Destroyed();

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
	}
}

void ASpatialDebugger::LoadIcons()
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	for (int i = 0; i < ICON_MAX; ++i)
	{
		UTexture2D* Texture = (UTexture2D*)StaticLoadObject(UTexture2D::StaticClass(), NULL, *IconFilenames[i], NULL, LOAD_NoWarn | LOAD_Quiet, NULL);
		Icons[i] = UCanvas::MakeIcon(Texture, 0.0f, 0.0f, 16.0f, 16.0f);
	}
}

void ASpatialDebugger::OnEntityAdded(const Worker_EntityId EntityId)
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	TWeakObjectPtr<AActor>* ExistingActor = EntityActorMapping.Find(EntityId);

	if (ExistingActor != nullptr)
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(Cast<USpatialNetDriver>(GetNetDriver())->PackageMap->GetObjectFromEntityId(EntityId).Get()))
	{
		EntityActorMapping.Add(EntityId, Actor);

		// Each client will only receive a PlayerController once
		if (Actor->IsA<APlayerController>())
		{
			LocalPlayerController = Cast<APlayerController>(Actor);
		}
	}
}

void ASpatialDebugger::OnEntityRemoved(const Worker_EntityId EntityId)
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	EntityActorMapping.Remove(EntityId);
}

void ASpatialDebugger::DrawTag(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawTag);

	if (bShowAuth)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
		// TODO: color code by worker id using WorkerColors array once that field is available
		const int WorkerId = 0;
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawIcon(Icons[ICON_AUTH], ScreenLocation.X - 32.0f, ScreenLocation.Y - 32.0f, 1.0f);
	}

	if (bShowAuthIntent)
	{
		const int VirtualWorkerId = VirtualWorkerIdToInt(GetVirtualWorkerId(EntityId));

		if (VirtualWorkerId != -1)
		{
			SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
			Canvas->SetDrawColor(WorkerColors[VirtualWorkerId]);
			Canvas->DrawIcon(Icons[ICON_AUTH_INTENT], ScreenLocation.X - 16.0f, ScreenLocation.Y - 32.0f, 1.0f);
		}
	}

	if (bShowLock)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
		// TODO: retrieve lock status once API is available
		const bool bIsLocked = false;
		const EIcon LockIcon = bIsLocked ? ICON_LOCK_CLOSED : ICON_LOCK_OPEN;

		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawIcon(Icons[LockIcon], ScreenLocation.X, ScreenLocation.Y - 32.0f, 1.0f);
	}

	FString Label;
	if (bShowEntityId)
	{
		SCOPE_CYCLE_COUNTER(STAT_BuildText);
		Label += FString::Printf(TEXT("%lld "), EntityId);
	}

	if (bShowActorName)
	{
		SCOPE_CYCLE_COUNTER(STAT_BuildText);
		Label += FString::Printf(TEXT("(%s)"), *ActorName);
	}

	if (bShowEntityId || bShowActorName)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawText);
		Canvas->SetDrawColor(FColor::Green);
		Canvas->DrawText(RenderFont, Label, ScreenLocation.X + 20.0f, ScreenLocation.Y - 32.0f, 1.0f, 1.0f, FontRenderInfo);
	}
}

void ASpatialDebugger::DrawDebug(UCanvas* Canvas, APlayerController* /* Controller */) // Controller is invalid
{
	SCOPE_CYCLE_COUNTER(STAT_DrawDebug);

	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	DrawDebugLocalPlayer(Canvas);

	FVector PlayerLocation = FVector::ZeroVector;

	if (LocalPawn)
	{
		PlayerLocation = LocalPawn->GetActorLocation();
	}

	ActorLocationCountMapping.Reset();

	for (auto& EntityActorPair : EntityActorMapping)
	{
		const TWeakObjectPtr<AActor> Actor = EntityActorPair.Value;
		const Worker_EntityId EntityId = (Worker_EntityId)EntityActorPair.Key;

		if (Actor != nullptr)
		{
			FVector ActorLocation = Actor->GetActorLocation();

			if (ActorLocation.IsZero())
			{
				continue;
			}

			if (FVector::Dist(PlayerLocation, ActorLocation) > MaxRange)
			{
				continue;
			}

			// If actors are very close together in world space, stack their tags vertically
			// TODO: maybe we want to do this in screen space instead...
			// TODO: fix the general jumpiness of tags with this turned on.
			int32& ActorCountAtLocation = ActorLocationCountMapping.FindOrAdd(HashPosition(ActorLocation));

			if (ActorCountAtLocation == 0)
			{
				FVector2D ScreenLocation = FVector2D::ZeroVector;
				{
					SCOPE_CYCLE_COUNTER(STAT_Projection);
					UGameplayStatics::ProjectWorldToScreen(LocalPlayerController, ActorLocation + FVector(0.0f, 0.0f, 200.0f), ScreenLocation, false);
				}

				DrawTag(Canvas, ScreenLocation - ActorCountAtLocation * FVector2D(0, STACKED_TAG_VERTICAL_OFFSET), EntityId, Actor->GetName());
			}

			if (bStack)
			{
				ActorCountAtLocation++;
			}
		}
	}
}

void ASpatialDebugger::DrawDebugLocalPlayer(UCanvas* Canvas)
{
	if (LocalPawn == nullptr ||
		LocalPlayerController == nullptr ||
		LocalPlayerState == nullptr)
	{
		return;
	}

	AActor* LocalPlayerActors[] =
	{
		LocalPawn,
		LocalPlayerController,
		LocalPlayerState
	};

	FVector2D ScreenLocation(PlayerPanelStartX, PlayerPanelStartY);

	for (int i = 0; i < sizeof(LocalPlayerActors)/sizeof(AActor*); ++i)
	{
		const USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		const Worker_EntityId EntityId = SpatialNetDriver->PackageMap->GetEntityIdFromObject(LocalPlayerActors[i]);

		DrawTag(Canvas, ScreenLocation, EntityId, LocalPlayerActors[i]->GetName());

		ScreenLocation.Y -= STACKED_TAG_VERTICAL_OFFSET;
	}
}

FString ASpatialDebugger::GetVirtualWorkerId(const Worker_EntityId EntityId) const
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	const AuthorityIntent* AuthorityIntentComponent = Cast<USpatialNetDriver>(GetNetDriver())->StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	if (AuthorityIntentComponent != nullptr)
	{
		return AuthorityIntentComponent->VirtualWorkerId;
	}

	return TEXT("INVALID VIRTUALWORKERID");
}

// TODO: When VirtualWorkerId is converted to int, this conversion function will no longer be necessary
int ASpatialDebugger::VirtualWorkerIdToInt(const FString& VirtualWorkerId)
{
	static const FString VirtualWorkerIds[VIRTUAL_WORKER_MAX_COUNT] =
	{
		"VW_A",
		"VW_B",
		"VW_C",
		"VW_D"
	};

	for (int i = 0; i < VIRTUAL_WORKER_MAX_COUNT; ++i)
	{
		if (VirtualWorkerIds[i].Equals(VirtualWorkerId.ToUpper()))
		{
			return i;
		}
	}

	return -1;
}

// Hashing Positions with hash algorithm from the following paper
// http://www.beosil.com/download/CollisionDetectionHashing_VMV03.pdf
// The FVector hash in Unreal is deprecated
int32 ASpatialDebugger::HashPosition(const FVector& P)
{
	const int32 p1 = 73856093;
	const int32 p2 = 19349663;
	const int32 p3 = 83492791;

	return ((int32)P.X * p1 ^ (int32)P.Y * p2 ^ (int32)P.Z * p3) % POSITION_HASH_BUCKET_RESERVATION_COUNT;
}

void ASpatialDebugger::SpatialToggleDebugger()
{
#if !UE_BUILD_SHIPPING
	check(GetNetDriver()->IsServer() == false);

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
		DrawDebugDelegateHandle.Reset();
	}
	else
	{
		DrawDebugDelegateHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &ASpatialDebugger::DrawDebug));
	}
#endif // !UE_BUILD_SHIPPING
}
