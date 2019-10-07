// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialDebugger.h"

#include "Debug/DebugDrawService.h"
#include "Engine/Engine.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Schema/AuthorityIntent.h"

using namespace SpatialGDK;

DEFINE_LOG_CATEGORY(LogSpatialDebugger);

#pragma optimize("", off)

ASpatialDebugger::ASpatialDebugger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxRange(100.0f * 100.0f)
	, bShowAuth(true)
	, bShowAuthIntent(true)
	, bShowLock(true)
	, bShowEntityId(true)
	, bShowActorName(true)
	, bAutoStart(false)
	, NetDriver(nullptr)
	, RenderFont(nullptr)
	, bActorSortRequired(false)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bAlwaysRelevant = true;
	bNetLoadOnClient = false;
	bReplicates = true;

	NetUpdateFrequency = 1.f;

	NetDriver = Cast<USpatialNetDriver>(GetNetDriver());

	// For GDK design reasons, this is the approach chosen to get a pointer
	// on the net driver to the client ASpatialDebugger.  Various alternatives
	// were considered and this is the best of a bad bunch.
	if (NetDriver &&
		NetDriver->IsServer() == false)
	{
		NetDriver->SetSpatialDebugger(this);
	}
}

void ASpatialDebugger::Tick(float DeltaSeconds)
{
	check(NetDriver != nullptr);

	if (NetDriver->IsServer() == false)
	{
		// Since we have no guarantee on the order we'll receive the PC/Pawn/PlayerState
		// over the wire, we check here once per tick (currently 1 Hz tick rate) to setup our local pointers.
		// Note that we can capture the PC in OnEntityAdded() since we know we will only receive one of those.
		if (LocalPawn.IsValid() == false && LocalPlayerController.IsValid())
		{
			LocalPawn = LocalPlayerController->GetPawn();
		}

		if (LocalPlayerState.IsValid() == false && LocalPawn.IsValid())
		{
			LocalPlayerState = LocalPawn->GetPlayerState();
		}

		if (LocalPawn.IsValid() && bActorSortRequired)
		{
			SCOPE_CYCLE_COUNTER(STAT_SortingActors);

			const FVector PlayerLocation = LocalPawn->GetActorLocation();

			EntityActorMapping.ValueSort([PlayerLocation](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B) {

				return FVector::Dist(PlayerLocation, A->GetActorLocation()) > FVector::Dist(PlayerLocation, B->GetActorLocation());
			});

			bActorSortRequired = false;
		}
	}
}

void ASpatialDebugger::BeginPlay()
{
	Super::BeginPlay();

	check(NetDriver != nullptr);

	if (NetDriver->IsServer() == false)
	{
		EntityActorMapping.Reserve(ENTITY_ACTOR_MAP_RESERVATION_COUNT);

		LoadIcons();

		TArray<Worker_EntityId_Key> EntityIds;
		NetDriver->StaticComponentView->GetEntityIds(EntityIds);

		// Capture any entities that are already present on this client (ie they came over the wire before the SpatialDebugger did)
		for (const Worker_EntityId_Key EntityId : EntityIds)
		{
			OnEntityAdded(EntityId);
		}

		// Register callbacks to get notified of all future entity arrivals / deletes
		OnEntityAddedHandle = NetDriver->Receiver->OnEntityAdded.AddUObject(this, &ASpatialDebugger::OnEntityAdded);
		OnEntityRemovedHandle = NetDriver->Receiver->OnEntityRemoved.AddUObject(this, &ASpatialDebugger::OnEntityRemoved);

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
	if (OnEntityAddedHandle.IsValid())
	{
		NetDriver->Receiver->OnEntityAdded.Remove(OnEntityAddedHandle);
	}

	if (OnEntityRemovedHandle.IsValid())
	{
		NetDriver->Receiver->OnEntityRemoved.Remove(OnEntityRemovedHandle);
	}

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
	}

	Super::Destroyed();
}

void ASpatialDebugger::LoadIcons()
{
	check(NetDriver != nullptr && NetDriver->IsServer() == false);

	const TArray<UTexture2D*> IconTextures = { AuthTexture, AuthIntentTexture, UnlockedTexture, LockedTexture };

	UTexture2D* DefaultTexture = nullptr;
	if (IconTextures.Contains(nullptr))
	{
		DefaultTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
		UE_LOG(LogSpatialDebugger, Warning, TEXT("At least one required icon texture has not been specified, will use default engine texture."));
	}

	for (int32 i = 0; i < ICON_MAX; ++i)
	{
		Icons[i] = UCanvas::MakeIcon(IconTextures[i] != nullptr ? IconTextures[i] : DefaultTexture, 0.0f, 0.0f, 16.0f, 16.0f);
	}
}

void ASpatialDebugger::OnEntityAdded(const Worker_EntityId EntityId)
{
	check(NetDriver != nullptr && NetDriver->IsServer() == false);

	TWeakObjectPtr<AActor>* ExistingActor = EntityActorMapping.Find(EntityId);

	if (ExistingActor != nullptr)
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId).Get()))
	{
		EntityActorMapping.Add(EntityId, Actor);

		// Each client will only receive a PlayerController once
		if (Actor->IsA<APlayerController>())
		{
			LocalPlayerController = Cast<APlayerController>(Actor);
		}

		bActorSortRequired = true;
	}
}

void ASpatialDebugger::OnEntityRemoved(const Worker_EntityId EntityId)
{
	check(NetDriver != nullptr && NetDriver->IsServer() == false);

	EntityActorMapping.Remove(EntityId);
	bActorSortRequired = true;
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
		const int32 VirtualWorkerId = GetVirtualWorkerId(EntityId);

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
		const EIcon LockIcon = bIsLocked ? ICON_LOCKED : ICON_UNLOCKED;

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

	check(NetDriver != nullptr && NetDriver->IsServer() == false);

	DrawDebugLocalPlayer(Canvas);

	FVector PlayerLocation = FVector::ZeroVector;

	if (LocalPawn.IsValid())
	{
		PlayerLocation = LocalPawn->GetActorLocation();
	}

	for (TPair<int64, TWeakObjectPtr<AActor>>& EntityActorPair : EntityActorMapping)
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

			FVector2D ScreenLocation = FVector2D::ZeroVector;
			if (LocalPlayerController.IsValid())
			{
				SCOPE_CYCLE_COUNTER(STAT_Projection);
				UGameplayStatics::ProjectWorldToScreen(LocalPlayerController.Get(), ActorLocation + FVector(0.0f, 0.0f, 200.0f), ScreenLocation, false);
			}

			DrawTag(Canvas, ScreenLocation, EntityId, Actor->GetName());
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

	TWeakObjectPtr<AActor> LocalPlayerActors[] =
	{
		LocalPawn,
		LocalPlayerController,
		LocalPlayerState
	};

	FVector2D ScreenLocation(PlayerPanelStartX, PlayerPanelStartY);

	for (int i = 0; i < sizeof(LocalPlayerActors)/sizeof(TWeakObjectPtr<AActor>); ++i)
	{
		if (LocalPlayerActors[i].IsValid())
		{
			const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(LocalPlayerActors[i].Get());

			DrawTag(Canvas, ScreenLocation, EntityId, LocalPlayerActors[i]->GetName());

			ScreenLocation.Y -= STACKED_TAG_VERTICAL_OFFSET;
		}
	}
}

// TODO: this should move to a shared location
int32 ASpatialDebugger::GetVirtualWorkerId(const Worker_EntityId EntityId) const
{
	check(NetDriver != nullptr && NetDriver->IsServer() == false);

	const AuthorityIntent* AuthorityIntentComponent = NetDriver->StaticComponentView->GetComponentData<AuthorityIntent>(EntityId);
	return (AuthorityIntentComponent != nullptr) ? AuthorityIntentComponent->VirtualWorkerId : SpatialConstants::INVALID_AUTHORITY_INTENT_ID;
}

void ASpatialDebugger::SpatialToggleDebugger()
{
#if !UE_BUILD_SHIPPING
	check(NetDriver != nullptr && NetDriver->IsServer() == false);

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

