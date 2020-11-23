// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialDebugger.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialStaticComponentView.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/WorkerRegion.h"
#include "Schema/SpatialDebugging.h"
#include "SpatialCommonTypes.h"
#include "Utils/InspectionColors.h"

#include "Debug/DebugDrawService.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "Net/UnrealNetwork.h"

using namespace SpatialGDK;

DEFINE_LOG_CATEGORY(LogSpatialDebugger);

namespace
{
// Background material for worker region
const FString DEFAULT_WORKER_REGION_MATERIAL =
	TEXT("/SpatialGDK/SpatialDebugger/Materials/TranslucentWorkerRegion.TranslucentWorkerRegion");
// Improbable primary font - Muli regular
const FString DEFAULT_WORKER_TEXT_FONT = TEXT("/SpatialGDK/SpatialDebugger/Fonts/MuliFont.MuliFont");
// Material to combine both the background and the worker information in one material
const FString DEFAULT_WORKER_COMBINED_MATERIAL =
	TEXT("/SpatialGDK/SpatialDebugger/Materials/WorkerRegionCombinedMaterial.WorkerRegionCombinedMaterial");
} // anonymous namespace

ASpatialDebugger::ASpatialDebugger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 1.f;

	bAlwaysRelevant = true;
	bNetLoadOnClient = false;
	bReplicates = true;

	NetUpdateFrequency = 1.f;

	NetDriver = Cast<USpatialNetDriver>(GetNetDriver());

	OnConfigUIClosed.BindDynamic(this, &ASpatialDebugger::DefaultOnConfigUIClosed);

	// For GDK design reasons, this is the approach chosen to get a pointer
	// on the net driver to the client ASpatialDebugger.  Various alternatives
	// were considered and this is the best of a bad bunch.
	if (NetDriver != nullptr && GetNetMode() == NM_Client)
	{
		NetDriver->SetSpatialDebugger(this);
	}
}

void ASpatialDebugger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASpatialDebugger, WorkerRegions, COND_SimulatedOnly);
}

void ASpatialDebugger::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	check(NetDriver != nullptr);

	if (!NetDriver->IsServer())
	{
		for (TMap<Worker_EntityId_Key, TWeakObjectPtr<AActor>>::TIterator It = EntityActorMapping.CreateIterator(); It; ++It)
		{
			if (!It->Value.IsValid())
			{
				It.RemoveCurrent();
			}
		}

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

		if (LocalPawn.IsValid())
		{
			SCOPE_CYCLE_COUNTER(STAT_SortingActors);
			const FVector& PlayerLocation = LocalPawn->GetActorLocation();

			EntityActorMapping.ValueSort([PlayerLocation](const TWeakObjectPtr<AActor>& A, const TWeakObjectPtr<AActor>& B) {
				return FVector::Dist(PlayerLocation, A->GetActorLocation()) > FVector::Dist(PlayerLocation, B->GetActorLocation());
			});
		}
	}
}

void ASpatialDebugger::BeginPlay()
{
	Super::BeginPlay();

	check(NetDriver != nullptr);

	if (!NetDriver->IsServer())
	{
		EntityActorMapping.Reserve(ENTITY_ACTOR_MAP_RESERVATION_COUNT);

		LoadIcons();

		TArray<Worker_EntityId_Key> EntityIds;
		NetDriver->StaticComponentView->GetEntityIds(EntityIds);

		// Capture any entities that are already present on this client (ie they came over the wire before the SpatialDebugger did).
		for (const Worker_EntityId_Key EntityId : EntityIds)
		{
			OnEntityAdded(EntityId);
		}

		// Register callbacks to get notified of all future entity arrivals / deletes.
		OnEntityAddedHandle = NetDriver->Receiver->OnEntityAddedDelegate.AddUObject(this, &ASpatialDebugger::OnEntityAdded);
		OnEntityRemovedHandle = NetDriver->Receiver->OnEntityRemovedDelegate.AddUObject(this, &ASpatialDebugger::OnEntityRemoved);

		FontRenderInfo.bClipText = true;
		FontRenderInfo.bEnableShadow = true;

		RenderFont = GEngine->GetSmallFont();

		if (bAutoStart)
		{
			SpatialToggleDebugger();
		}
	}
}

void ASpatialDebugger::OnAuthorityGained()
{
	if (UAbstractLBStrategy* LoadBalanceStrategy = Cast<UAbstractLBStrategy>(NetDriver->LoadBalanceStrategy))
	{
		if (const UGridBasedLBStrategy* GridBasedLBStrategy =
				Cast<UGridBasedLBStrategy>(LoadBalanceStrategy->GetLBStrategyForVisualRendering()))
		{
			const UGridBasedLBStrategy::LBStrategyRegions LBStrategyRegions = GridBasedLBStrategy->GetLBStrategyRegions();
			WorkerRegions.SetNum(LBStrategyRegions.Num());
			for (int i = 0; i < LBStrategyRegions.Num(); i++)
			{
				FWorkerRegionInfo WorkerRegionInfo;
				const TPair<VirtualWorkerId, FBox2D>& LBStrategyRegion = LBStrategyRegions[i];
				WorkerRegionInfo.VirtualWorkerID = LBStrategyRegion.Key;
				const PhysicalWorkerName* WorkerName =
					NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(LBStrategyRegion.Key);
				WorkerRegionInfo.WorkerName = (WorkerName == nullptr) ? "" : *WorkerName;
				WorkerRegionInfo.Color = (WorkerName == nullptr) ? InvalidServerTintColor : SpatialGDK::GetColorForWorkerName(*WorkerName);
				WorkerRegionInfo.Extents = LBStrategyRegion.Value;
				WorkerRegions[i] = WorkerRegionInfo;
			}
		}
	}
}

void ASpatialDebugger::CreateWorkerRegions()
{
	UMaterial* WorkerRegionMaterial = LoadObject<UMaterial>(nullptr, *DEFAULT_WORKER_REGION_MATERIAL);
	if (WorkerRegionMaterial == nullptr)
	{
		UE_LOG(LogSpatialDebugger, Error, TEXT("Worker regions were not rendered. Could not find default material: %s"),
			   *DEFAULT_WORKER_REGION_MATERIAL);
		return;
	}

	UMaterial* WorkerCombinedMaterial = LoadObject<UMaterial>(nullptr, *DEFAULT_WORKER_COMBINED_MATERIAL);
	if (WorkerCombinedMaterial == nullptr)
	{
		UE_LOG(LogSpatialDebugger, Error, TEXT("Worker regions were not rendered. Could not find default material: %s"),
			   *DEFAULT_WORKER_COMBINED_MATERIAL);
	}

	UFont* WorkerInfoFont = LoadObject<UFont>(nullptr, *DEFAULT_WORKER_TEXT_FONT);
	if (WorkerInfoFont == nullptr)
	{
		UE_LOG(LogSpatialDebugger, Error, TEXT("Worker information was not rendered. Could not find default font: %s"),
			   *DEFAULT_WORKER_TEXT_FONT);
	}

	// Create new actors for all new worker regions
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	UWorld* World = GetWorld();
#if WITH_EDITOR
	if (World == nullptr)
	{
		// We are in the editor at design time
		World = GEditor->GetEditorWorldContext().World();
	}
	SpawnParams.bHideFromSceneOutliner = true;
#endif
	check(World != nullptr);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FWorkerRegionInfo& WorkerRegionData : WorkerRegions)
	{
		AWorkerRegion* WorkerRegion = World->SpawnActor<AWorkerRegion>(SpawnParams);
		FString WorkerInfo = FString::Printf(TEXT("You are looking at virtual worker number %d\n%s"), WorkerRegionData.VirtualWorkerID,
											 *WorkerRegionData.WorkerName);
		WorkerRegion->Init(WorkerRegionMaterial, WorkerCombinedMaterial, WorkerInfoFont, WorkerRegionData.Color, WorkerRegionOpacity,
						   WorkerRegionData.Extents, WorkerRegionHeight, WorkerRegionVerticalScale, WorkerInfo);
		WorkerRegion->SetActorEnableCollision(false);
	}
}

void ASpatialDebugger::DestroyWorkerRegions()
{
	TArray<AActor*> WorkerRegionsToRemove;
	UGameplayStatics::GetAllActorsOfClass(this, AWorkerRegion::StaticClass(), WorkerRegionsToRemove);
	for (AActor* WorkerRegion : WorkerRegionsToRemove)
	{
		WorkerRegion->Destroy();
	}
}

void ASpatialDebugger::OnRep_SetWorkerRegions()
{
	if (NetDriver != nullptr && !NetDriver->IsServer() && DrawDebugDelegateHandle.IsValid() && bShowWorkerRegions)
	{
		DestroyWorkerRegions();
		CreateWorkerRegions();
	}
}

void ASpatialDebugger::Destroyed()
{
	if (NetDriver != nullptr && NetDriver->Receiver != nullptr)
	{
		if (OnEntityAddedHandle.IsValid())
		{
			NetDriver->Receiver->OnEntityAddedDelegate.Remove(OnEntityAddedHandle);
		}

		if (OnEntityRemovedHandle.IsValid())
		{
			NetDriver->Receiver->OnEntityRemovedDelegate.Remove(OnEntityRemovedHandle);
		}
	}

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
	}

	DestroyWorkerRegions();

	Super::Destroyed();
}

void ASpatialDebugger::LoadIcons()
{
	check(NetDriver != nullptr && !NetDriver->IsServer());

	UTexture2D* DefaultTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));

	const float IconWidth = 16.0f;
	const float IconHeight = 16.0f;

	Icons[ICON_AUTH] = UCanvas::MakeIcon(AuthTexture != nullptr ? AuthTexture : DefaultTexture, 0.0f, 0.0f, IconWidth, IconHeight);
	Icons[ICON_AUTH_INTENT] =
		UCanvas::MakeIcon(AuthIntentTexture != nullptr ? AuthIntentTexture : DefaultTexture, 0.0f, 0.0f, IconWidth, IconHeight);
	Icons[ICON_UNLOCKED] =
		UCanvas::MakeIcon(UnlockedTexture != nullptr ? UnlockedTexture : DefaultTexture, 0.0f, 0.0f, IconWidth, IconHeight);
	Icons[ICON_LOCKED] = UCanvas::MakeIcon(LockedTexture != nullptr ? LockedTexture : DefaultTexture, 0.0f, 0.0f, IconWidth, IconHeight);
	Icons[ICON_BOX] = UCanvas::MakeIcon(BoxTexture != nullptr ? BoxTexture : DefaultTexture, 0.0f, 0.0f, IconWidth, IconHeight);
}

void ASpatialDebugger::OnEntityAdded(const Worker_EntityId EntityId)
{
	check(NetDriver != nullptr && !NetDriver->IsServer());

	TWeakObjectPtr<AActor>* ExistingActor = EntityActorMapping.Find(EntityId);

	if (ExistingActor != nullptr)
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId).Get()))
	{
		EntityActorMapping.Add(EntityId, Actor);

		// Each client will only receive a PlayerController once.
		if (Actor->IsA<APlayerController>())
		{
			LocalPlayerController = Cast<APlayerController>(Actor);

			if (GetNetMode() == NM_Client)
			{
				LocalPlayerController->InputComponent->BindKey(ConfigUIToggleKey, IE_Pressed, this, &ASpatialDebugger::OnToggleConfigUI);
			}
		}
	}
}

void ASpatialDebugger::OnToggleConfigUI()
{
	if (ConfigUIWidget == nullptr)
	{
		if (ConfigUIClass != nullptr)
		{
			ConfigUIWidget = CreateWidget<USpatialDebuggerConfigUI>(LocalPlayerController.Get(), ConfigUIClass);
			if (ConfigUIWidget == nullptr)
			{
				UE_LOG(LogSpatialDebugger, Error,
					   TEXT("SpatialDebugger config UI will not load. Couldn't create config UI widget for class: %s"),
					   *GetNameSafe(ConfigUIClass));
				return;
			}
			else
			{
				ConfigUIWidget->SetSpatialDebugger(this);
			}
		}
		else
		{
			UE_LOG(LogSpatialDebugger, Error,
				   TEXT("SpatialDebugger config UI will not load. ConfigUIClass is not set on the spatial debugger."));
			return;
		}

		ConfigUIWidget->AddToViewport();

		FInputModeGameAndUI InputModeSettings;
		InputModeSettings.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeSettings.SetWidgetToFocus(ConfigUIWidget->TakeWidget());

		LocalPlayerController->SetInputMode(InputModeSettings);
		LocalPlayerController->bShowMouseCursor = true;

		ConfigUIWidget->OnShow();
	}
	else
	{
		ConfigUIWidget->RemoveFromParent();
		ConfigUIWidget = nullptr;
		OnConfigUIClosed.ExecuteIfBound();
	}
}

void ASpatialDebugger::DefaultOnConfigUIClosed()
{
	if (LocalPlayerController.IsValid())
	{
		FInputModeGameOnly InputModeSettings;
		LocalPlayerController->SetInputMode(InputModeSettings);
		LocalPlayerController->bShowMouseCursor = false;
	}
}

void ASpatialDebugger::SetShowWorkerRegions(const bool bNewShow)
{
	if (bNewShow != bShowWorkerRegions)
	{
		if (IsEnabled())
		{
			if (bNewShow)
			{
				CreateWorkerRegions();
			}
			else
			{
				DestroyWorkerRegions();
			}
		}

		bShowWorkerRegions = bNewShow;
	}
}

void ASpatialDebugger::OnEntityRemoved(const Worker_EntityId EntityId)
{
	check(NetDriver != nullptr && !NetDriver->IsServer());

	EntityActorMapping.Remove(EntityId);
}

void ASpatialDebugger::ActorAuthorityChanged(const Worker_ComponentSetAuthorityChangeOp& AuthOp) const
{
	check(AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE && AuthOp.component_set_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID);

	if (NetDriver->VirtualWorkerTranslator == nullptr)
	{
		// Currently, there's nothing to display in the debugger other than load balancing information.
		return;
	}

	const VirtualWorkerId LocalVirtualWorkerId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();
	const FColor LocalVirtualWorkerColor =
		SpatialGDK::GetColorForWorkerName(NetDriver->VirtualWorkerTranslator->GetLocalPhysicalWorkerName());

	SpatialDebugging* DebuggingInfo = NetDriver->StaticComponentView->GetComponentData<SpatialDebugging>(AuthOp.entity_id);
	if (DebuggingInfo == nullptr)
	{
		// Some entities won't have debug info, so create it now.
		SpatialDebugging NewDebuggingInfo(LocalVirtualWorkerId, LocalVirtualWorkerColor, SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
										  InvalidServerTintColor, false);
		NetDriver->Sender->SendAddComponents(AuthOp.entity_id, { NewDebuggingInfo.CreateSpatialDebuggingData() });
		return;
	}

	DebuggingInfo->AuthoritativeVirtualWorkerId = LocalVirtualWorkerId;
	DebuggingInfo->AuthoritativeColor = LocalVirtualWorkerColor;
	FWorkerComponentUpdate DebuggingUpdate = DebuggingInfo->CreateSpatialDebuggingUpdate();
	NetDriver->Connection->SendComponentUpdate(AuthOp.entity_id, &DebuggingUpdate);
}

void ASpatialDebugger::ActorAuthorityIntentChanged(Worker_EntityId EntityId, VirtualWorkerId NewIntentVirtualWorkerId) const
{
	SpatialDebugging* DebuggingInfo = NetDriver->StaticComponentView->GetComponentData<SpatialDebugging>(EntityId);
	check(DebuggingInfo != nullptr);
	DebuggingInfo->IntentVirtualWorkerId = NewIntentVirtualWorkerId;

	const PhysicalWorkerName* NewAuthoritativePhysicalWorkerName =
		NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(NewIntentVirtualWorkerId);
	check(NewAuthoritativePhysicalWorkerName != nullptr);

	DebuggingInfo->IntentColor = SpatialGDK::GetColorForWorkerName(*NewAuthoritativePhysicalWorkerName);
	FWorkerComponentUpdate DebuggingUpdate = DebuggingInfo->CreateSpatialDebuggingUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &DebuggingUpdate);
}

void ASpatialDebugger::DrawTag(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName,
							   const bool bCentre)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawTag);

	check(NetDriver != nullptr && !NetDriver->IsServer());
	if (!NetDriver->StaticComponentView->HasComponent(EntityId, SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID))
	{
		return;
	}

	const SpatialDebugging* DebuggingInfo = NetDriver->StaticComponentView->GetComponentData<SpatialDebugging>(EntityId);

	if (!FApp::CanEverRender()) // DrawIcon can attempt to use the underlying texture resource even when using nullrhi
	{
		return;
	}

	static const float BaseHorizontalOffset = 16.0f;
	static const float NumberScale = 0.75f;
	static const float TextScale = 0.5f;
	const float AuthIdWidth = NumberScale * GetNumberOfDigitsIn(DebuggingInfo->AuthoritativeVirtualWorkerId);
	const float AuthIntentIdWidth = NumberScale * GetNumberOfDigitsIn(DebuggingInfo->IntentVirtualWorkerId);
	const float EntityIdWidth = NumberScale * GetNumberOfDigitsIn(EntityId);

	int32 HorizontalOffset = 0;
	if (bCentre)
	{
		// If tag should be centered, calculate the total width of the icons and text to be rendered
		float TagWidth = 0;
		if (bShowLock)
		{
			// If showing the lock, add the lock icon width
			TagWidth += BaseHorizontalOffset;
		}
		if (bShowAuth)
		{
			// If showing the authority, add the authority icon width and the width of the authoritative virtual worker ID
			TagWidth += BaseHorizontalOffset;
			TagWidth += (BaseHorizontalOffset * AuthIdWidth);
		}
		if (bShowAuthIntent)
		{
			// If showing the authority intent, add the authority intent icon width and the width of the authoritative intent virtual worker
			// ID
			TagWidth += BaseHorizontalOffset;
			TagWidth += (BaseHorizontalOffset * AuthIntentIdWidth);
		}
		if (bShowEntityId)
		{
			// If showing the entity ID, add the width of the entity ID
			TagWidth += (BaseHorizontalOffset * EntityIdWidth);
		}
		if (bShowActorName)
		{
			// If showing the actor name, add the width of the actor name
			const float ActorNameWidth = TextScale * ActorName.Len();
			TagWidth += (BaseHorizontalOffset * ActorNameWidth);
		}

		// Calculate the offset based on the total width of the tag
		HorizontalOffset = TagWidth / -2;
	}

	// Draw icons and text based on the offset
	if (bShowLock)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
		const bool bIsLocked = DebuggingInfo->IsLocked;
		const EIcon LockIcon = bIsLocked ? ICON_LOCKED : ICON_UNLOCKED;

		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawIcon(Icons[LockIcon], ScreenLocation.X + HorizontalOffset, ScreenLocation.Y, 1.0f);
		HorizontalOffset += BaseHorizontalOffset;
	}

	if (bShowAuth)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
		const FColor& ServerWorkerColor = DebuggingInfo->AuthoritativeColor;
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawIcon(Icons[ICON_AUTH], ScreenLocation.X + HorizontalOffset, ScreenLocation.Y, 1.0f);
		HorizontalOffset += BaseHorizontalOffset;
		Canvas->SetDrawColor(ServerWorkerColor);
		Canvas->DrawScaledIcon(Icons[ICON_BOX], ScreenLocation.X + HorizontalOffset, ScreenLocation.Y, FVector(AuthIdWidth, 1.f, 1.f));
		Canvas->SetDrawColor(GetTextColorForBackgroundColor(ServerWorkerColor));
		Canvas->DrawText(RenderFont, FString::FromInt(DebuggingInfo->AuthoritativeVirtualWorkerId), ScreenLocation.X + HorizontalOffset + 1,
						 ScreenLocation.Y, 1.1f, 1.1f, FontRenderInfo);
		HorizontalOffset += (BaseHorizontalOffset * AuthIdWidth);
	}

	if (bShowAuthIntent)
	{
		SCOPE_CYCLE_COUNTER(STAT_DrawIcons);
		const FColor& VirtualWorkerColor = DebuggingInfo->IntentColor;
		Canvas->SetDrawColor(FColor::White);
		Canvas->DrawIcon(Icons[ICON_AUTH_INTENT], ScreenLocation.X + HorizontalOffset, ScreenLocation.Y, 1.0f);
		HorizontalOffset += BaseHorizontalOffset;
		Canvas->SetDrawColor(VirtualWorkerColor);
		Canvas->DrawScaledIcon(Icons[ICON_BOX], ScreenLocation.X + HorizontalOffset, ScreenLocation.Y,
							   FVector(AuthIntentIdWidth, 1.f, 1.f));
		Canvas->SetDrawColor(GetTextColorForBackgroundColor(VirtualWorkerColor));
		Canvas->DrawText(RenderFont, FString::FromInt(DebuggingInfo->IntentVirtualWorkerId), ScreenLocation.X + HorizontalOffset + 1,
						 ScreenLocation.Y, 1.1f, 1.1f, FontRenderInfo);
		HorizontalOffset += (BaseHorizontalOffset * AuthIntentIdWidth);
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
		Canvas->DrawText(RenderFont, Label, ScreenLocation.X + HorizontalOffset, ScreenLocation.Y, 1.0f, 1.0f, FontRenderInfo);
	}
}

FColor ASpatialDebugger::GetTextColorForBackgroundColor(const FColor& BackgroundColor) const
{
	return BackgroundColor.ReinterpretAsLinear().ComputeLuminance() > 0.5 ? FColor::Black : FColor::White;
}

// This will break once we have more than 10,000 workers, happily kicking that can down the road.
int32 ASpatialDebugger::GetNumberOfDigitsIn(int32 SomeNumber) const
{
	SomeNumber = FMath::Abs(SomeNumber);
	return (SomeNumber < 10 ? 1 : (SomeNumber < 100 ? 2 : (SomeNumber < 1000 ? 3 : 4)));
}

void ASpatialDebugger::DrawDebug(UCanvas* Canvas, APlayerController* /* Controller */) // Controller is invalid.
{
	SCOPE_CYCLE_COUNTER(STAT_DrawDebug);

	check(NetDriver != nullptr && !NetDriver->IsServer());

#if WITH_EDITOR
	// Prevent one client's data rendering in another client's view in PIE when using UDebugDrawService.  Lifted from EQSRenderingComponent.
	if (Canvas && Canvas->SceneView && Canvas->SceneView->Family && Canvas->SceneView->Family->Scene
		&& Canvas->SceneView->Family->Scene->GetWorld() != GetWorld())
	{
		return;
	}
#endif

	if (ActorTagDrawMode >= EActorTagDrawMode::LocalPlayer)
	{
		DrawDebugLocalPlayer(Canvas);
	}

	FVector PlayerLocation = FVector::ZeroVector;

	if (ActorTagDrawMode == EActorTagDrawMode::All)
	{
		if (LocalPawn.IsValid())
		{
			PlayerLocation = LocalPawn->GetActorLocation();
		}

		for (TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>& EntityActorPair : EntityActorMapping)
		{
			const TWeakObjectPtr<AActor> Actor = EntityActorPair.Value;
			const Worker_EntityId EntityId = EntityActorPair.Key;

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
					UGameplayStatics::ProjectWorldToScreen(LocalPlayerController.Get(), ActorLocation + WorldSpaceActorTagOffset,
														   ScreenLocation, false);
				}

				if (ScreenLocation.IsZero())
				{
					continue;
				}

				DrawTag(Canvas, ScreenLocation, EntityId, Actor->GetName(), true /*bCentre*/);
			}
		}
	}
}

void GetReplicatedActorsInHierarchy(const AActor* Actor, TArray<const AActor*>& HierarchyActors)
{
	if (Actor->GetIsReplicated() && !HierarchyActors.Contains(Actor))
	{
		HierarchyActors.Add(Actor);
	}
	for (const AActor* Child : Actor->Children)
	{
		GetReplicatedActorsInHierarchy(Child, HierarchyActors);
	}
}

void ASpatialDebugger::DrawDebugLocalPlayer(UCanvas* Canvas)
{
	if (LocalPawn == nullptr || LocalPlayerController == nullptr || LocalPlayerState == nullptr)
	{
		return;
	}

	TArray<const AActor*> ActorsToDisplay = { LocalPlayerState.Get(), LocalPlayerController.Get(), LocalPawn.Get() };

	if (bShowPlayerHierarchy)
	{
		GetReplicatedActorsInHierarchy(LocalPlayerController.Get(), ActorsToDisplay);
	}

	FVector2D ScreenLocation(PlayerPanelStartX, PlayerPanelStartY);

	for (int32 i = 0; i < ActorsToDisplay.Num(); ++i)
	{
		const Worker_EntityId EntityId = NetDriver->PackageMap->GetEntityIdFromObject(ActorsToDisplay[i]);
		DrawTag(Canvas, ScreenLocation, EntityId, ActorsToDisplay[i]->GetName(), false /*bCentre*/);
		ScreenLocation.Y += PLAYER_TAG_VERTICAL_OFFSET;
	}
}

void ASpatialDebugger::SpatialToggleDebugger()
{
	check(NetDriver != nullptr && !NetDriver->IsServer());

	if (DrawDebugDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DrawDebugDelegateHandle);
		DrawDebugDelegateHandle.Reset();
		DestroyWorkerRegions();
	}
	else
	{
		DrawDebugDelegateHandle =
			UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateUObject(this, &ASpatialDebugger::DrawDebug));
		if (bShowWorkerRegions)
		{
			CreateWorkerRegions();
		}
	}
}

bool ASpatialDebugger::IsEnabled()
{
	return DrawDebugDelegateHandle.IsValid();
}

#if WITH_EDITOR
void ASpatialDebugger::EditorRefreshDisplay()
{
	if (GEditor != nullptr && GEditor->GetActiveViewport() != nullptr)
	{
		// Redraw editor window to show changes
		GEditor->GetActiveViewport()->Invalidate();
	}
}

void ASpatialDebugger::EditorSpatialToggleDebugger(bool bEnabled)
{
	bShowWorkerRegions = bEnabled;
	EditorRefreshWorkerRegions();
}

void ASpatialDebugger::EditorRefreshWorkerRegions()
{
	DestroyWorkerRegions();

	if (bShowWorkerRegions && EditorAllowWorkerBoundaries())
	{
		EditorInitialiseWorkerRegions();
		CreateWorkerRegions();
	}

	EditorRefreshDisplay();
}

bool ASpatialDebugger::EditorAllowWorkerBoundaries() const
{
	// Check if spatial networking is enabled.
	return GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
}

void ASpatialDebugger::EditorInitialiseWorkerRegions()
{
	WorkerRegions.Empty();

	const UWorld* World = GEditor->GetEditorWorldContext().World();
	check(World != nullptr);

	const UAbstractSpatialMultiWorkerSettings* MultiWorkerSettings =
		USpatialStatics::GetSpatialMultiWorkerClass(World)->GetDefaultObject<UAbstractSpatialMultiWorkerSettings>();

	ULayeredLBStrategy* LoadBalanceStrategy = NewObject<ULayeredLBStrategy>();
	LoadBalanceStrategy->Init();
	LoadBalanceStrategy->SetLayers(MultiWorkerSettings->WorkerLayers);

	if (const UGridBasedLBStrategy* GridBasedLBStrategy =
			Cast<UGridBasedLBStrategy>(LoadBalanceStrategy->GetLBStrategyForVisualRendering()))
	{
		LoadBalanceStrategy->SetVirtualWorkerIds(1, LoadBalanceStrategy->GetMinimumRequiredWorkers());
		const UGridBasedLBStrategy::LBStrategyRegions LBStrategyRegions = GridBasedLBStrategy->GetLBStrategyRegions();

		WorkerRegions.SetNum(LBStrategyRegions.Num());
		for (int i = 0; i < LBStrategyRegions.Num(); i++)
		{
			const TPair<VirtualWorkerId, FBox2D>& LBStrategyRegion = LBStrategyRegions[i];
			FWorkerRegionInfo WorkerRegionInfo;
			// Generate our own unique worker name as we only need it to generate a unique colour
			const PhysicalWorkerName WorkerName = PhysicalWorkerName::Printf(TEXT("WorkerRegion%d%d%d"), i, i, i);
			WorkerRegionInfo.Color = GetColorForWorkerName(WorkerName);
			WorkerRegionInfo.Extents = LBStrategyRegion.Value;
			WorkerRegions[i] = WorkerRegionInfo;
		}
	}

	// Needed to clean up LoadBalanceStrategy memory, otherwise it gets duplicated exponentially
	GEngine->ForceGarbageCollection(true);
}

void ASpatialDebugger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialDebugger, WorkerRegionHeight)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialDebugger, WorkerRegionVerticalScale)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(ASpatialDebugger, WorkerRegionOpacity))

		{
			EditorRefreshWorkerRegions();
		}
	}
}
#endif // WITH_EDITOR
