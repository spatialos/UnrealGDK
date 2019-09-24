// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialDebugger.h"

#include "Components/SceneComponent.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Engine.h"
#include "EngineClasses/SpatialLoadBalancingStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerController.h"
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
	, LocalPawn(nullptr)
	, LocalPlayerController(nullptr)
	, MaxRange(100.0f * 100.0f)
	, bShowAuth(true)
	, bShowAuthIntent(true)
	, bShowLock(true)
	, bShowEntityId(true)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 1.f;
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
	}
}

void ASpatialDebugger::OnEntityRemoved(const Worker_EntityId EntityId)
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	EntityActorMapping.Remove(EntityId);
}

void ASpatialDebugger::Tick(float DeltaSeconds)
{
	if (GetNetDriver() &&
		GetNetDriver()->IsServer() == false)
	{
		if (!LocalPlayerController)
		{
			if (const UNetDriver* LocalNetDriver = GetNetDriver())
			{
				if (const UWorld* World = LocalNetDriver->GetWorld())
				{
					LocalPlayerController = World->GetFirstPlayerController();
				}
			}
		}

		if (LocalPlayerController && !LocalPawn)
		{
			LocalPawn = LocalPlayerController->GetPawn();
		}
	}
}

void ASpatialDebugger::BeginPlay()
{
	Super::BeginPlay();

	check(GetNetDriver() != nullptr);

	if (!GetNetDriver()->IsServer())
	{
		LoadIcons();

		DrawDebugDelegateHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &ASpatialDebugger::DrawDebug));

		TArray<Worker_EntityId_Key> EntityIds;
		Cast<USpatialNetDriver>(GetNetDriver())->StaticComponentView->GetEntityIds(EntityIds);

		for (const Worker_EntityId_Key EntityId : EntityIds)
		{
			OnEntityAdded(EntityId);
		}

		Cast<USpatialNetDriver>(GetNetDriver())->Receiver->OnEntityAdded.BindUObject(this, &ASpatialDebugger::OnEntityAdded);
		Cast<USpatialNetDriver>(GetNetDriver())->Receiver->OnEntityRemoved.BindUObject(this, &ASpatialDebugger::OnEntityRemoved);
	}
}

void ASpatialDebugger::DrawDebug(class UCanvas* Canvas, APlayerController* Controller)
{
	check(GetNetDriver() != nullptr);
	check(GetNetDriver()->IsServer() == false);

	FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);
	UFont* RenderFont = GEngine->GetSmallFont();

	TMap<float, int32> ActorsPerLocation;
	ActorsPerLocation.Reserve(POSITION_HASH_BUCKETS);

	FVector PlayerLocation = FVector::ZeroVector;

	if (LocalPawn)
	{
		PlayerLocation = LocalPawn->GetActorLocation();
	}

	for (auto& EntityActorPair : EntityActorMapping)
	{
		const TWeakObjectPtr<AActor> Actor = EntityActorPair.Value;

		if (Actor != nullptr)
		{
			USceneComponent* SceneComponent = Cast<USceneComponent>(Actor->GetComponentByClass(USceneComponent::StaticClass()));

			if (SceneComponent == nullptr ||
				SceneComponent->IsVisible() == false)
			{
				continue;
			}

			FVector ActorLocation = Actor->GetActorLocation();

			if (ActorLocation.IsZero())
			{
				continue;
			}

			//if (FVector::Dist(PlayerLocation, ActorLocation) > MaxRange)
			//{
			//	continue;
			//}

			// If actors are very close together in world space, stack their icons/entityids vertically
			// TODO: maybe we want to do this in screen space instead...
			// TODO: fix the jumpiness on server worker transition...disabled for now.
			//int32& ActorCountAtLocation = ActorsPerLocation.FindOrAdd(HashPosition(ActorLocation));
			int32 ActorCountAtLocation = 0;

			FVector2D ScreenLocation = FVector2D::ZeroVector;
			UGameplayStatics::ProjectWorldToScreen(LocalPlayerController, ActorLocation + FVector(0.0f, 0.0f, 200.0f), ScreenLocation, false);

			if (bShowAuth)
			{
				// TODO: color code by worker id using WorkerColors array once that field is available
				const int WorkerId = 0;
				Canvas->SetDrawColor(FColor::White);
				Canvas->DrawIcon(Icons[ICON_AUTH], ScreenLocation.X - 32.0f, ScreenLocation.Y - ActorCountAtLocation * 32.0f, 1.0f);
			}

			if (bShowAuthIntent)
			{
				const int VirtualWorkerId = VirtualWorkerIdToInt(GetVirtualWorkerId((Worker_EntityId)EntityActorPair.Key));
				check(VirtualWorkerId != -1);

				Canvas->SetDrawColor(WorkerColors[VirtualWorkerId]);
				Canvas->DrawIcon(Icons[ICON_AUTH_INTENT], ScreenLocation.X - 16.0f, ScreenLocation.Y - ActorCountAtLocation * 32.0f, 1.0f);
			}

			if (bShowLock)
			{
				// TODO: retrieve lock status once API is available
				const bool bIsLocked = false;
				const ELockStatus LockStatus = bIsLocked ? LOCKSTATUS_CLOSED : LOCKSTATUS_OPEN;
				const EIcon LockIcon = bIsLocked ? ICON_LOCK_CLOSED : ICON_LOCK_OPEN;

				//Canvas->SetDrawColor(LockColors[LockStatus]);
				Canvas->SetDrawColor(FColor::White);
				Canvas->DrawIcon(Icons[LockIcon], ScreenLocation.X, ScreenLocation.Y - ActorCountAtLocation * 32.0f, 1.0f);
			}

			if (bShowEntityId)
			{
				Canvas->SetDrawColor(FColor::Green);
				Canvas->DrawText(RenderFont, FString::Printf(TEXT("%lld"), EntityActorPair.Key), ScreenLocation.X + 16.0f, ScreenLocation.Y - ActorCountAtLocation * 32.0f, 1.0f, 1.0f, FontRenderInfo);
			}
		}
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
		GetNetDriver()->LogMe(FString::Format(TEXT("Loaded Icon {0}"), { *IconFilenames[i] }));
	}
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

// Hashing Positions with hash algorithm from the following paper
// http://www.beosil.com/download/CollisionDetectionHashing_VMV03.pdf
// The FVector hash in Unreal is deprecated
int32 ASpatialDebugger::HashPosition(const FVector& P)
{
	const int32 p1 = 73856093;
	const int32 p2 = 19349663;
	const int32 p3 = 83492791;

	return ((int32)P.X * p1 ^ (int32)P.Y * p2 ^ (int32)P.Z * p3) % POSITION_HASH_BUCKETS;
}
