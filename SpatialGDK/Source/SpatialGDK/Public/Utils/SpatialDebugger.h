// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/WorkerRegion.h"
#include "SpatialCommonTypes.h"
#include "SpatialDebuggerConfigUI.h"

#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "GameFramework/Info.h"
#include "Materials/Material.h"
#include "Math/Box2D.h"
#include "Math/Color.h"
#include "Templates/Tuple.h"

#include <WorkerSDK/improbable/c_worker.h>
#include "SpatialDebugger.generated.h"

class APawn;
class APlayerController;
class APlayerState;
class USpatialNetDriver;
class UFont;
class UTexture2D;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialDebugger, Log, All);

DECLARE_STATS_GROUP(TEXT("SpatialDebugger"), STATGROUP_SpatialDebugger, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("DrawDebug"), STAT_DrawDebug, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawTag"), STAT_DrawTag, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("Projection"), STAT_Projection, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawIcons"), STAT_DrawIcons, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawText"), STAT_DrawText, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("BuildText"), STAT_BuildText, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("SortingActors"), STAT_SortingActors, STATGROUP_SpatialDebugger);

USTRUCT()
struct FWorkerRegionInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FColor Color;

	UPROPERTY()
	FBox2D Extents;

	UPROPERTY()
	FString WorkerName;

	UPROPERTY()
	uint32 VirtualWorkerID;
};

UENUM()
namespace EActorTagDrawMode
{
enum Type
{
	None,
	LocalPlayer,
	All
};
} // namespace EActorTagDrawMode

DECLARE_DYNAMIC_DELEGATE(FOnConfigUIClosedDelegate);

/**
 * Visualise spatial information at runtime and in the editor
 */
UCLASS(SpatialType = (NotPersistent), Blueprintable, NotPlaceable, Transient)
class SPATIALGDK_API ASpatialDebugger : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	virtual void OnAuthorityGained() override;

	UFUNCTION(Exec, Category = "SpatialGDK", BlueprintCallable)
	void SpatialToggleDebugger();

	UFUNCTION(Category = "SpatialGDK", BlueprintCallable, BlueprintPure)
	bool IsEnabled();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI,
			  meta = (ToolTip = "Key to open configuration UI for the debugger at runtime"))
	FKey ConfigUIToggleKey = EKeys::F9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (ToolTip = "Key to select actor when debugging in game"))
	FKey SelectActorKey = EKeys::RightMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI,
			  meta = (ToolTip = "Key to highlight next actor under cursor when debugging in game"))
	FKey HighlightActorKey = EKeys::MouseWheelAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (ToolTip = "In-game configuration UI widget"))
	TSubclassOf<USpatialDebuggerConfigUI> ConfigUIClass;

	FOnConfigUIClosedDelegate OnConfigUIClosed;

	// TODO: Expose these through a runtime UI: https://improbableio.atlassian.net/browse/UNR-2359.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "X location of player data panel"))
	int PlayerPanelStartX = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "Y location of player data panel"))
	int PlayerPanelStartY = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General,
			  meta = (ToolTip = "Maximum range from local player that tags will be drawn out to"))
	float MaxRange = 100.0f * 100.0f; // 100m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (Tooltip = "Which Actor tags to show"))
	TEnumAsByte<EActorTagDrawMode::Type> ActorTagDrawMode = EActorTagDrawMode::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (Tooltip = "Show all replicated Actors in the player controller's hierarchy, or just state/controller/pawn"))
	bool bShowPlayerHierarchy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Show server authority for every entity in range"))
	bool bShowAuth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Show authority intent for every entity in range"))
	bool bShowAuthIntent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show lock status for every entity in range"))
	bool bShowLock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show EntityId for every entity in range"))
	bool bShowEntityId = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show Actor Name for every entity in range"))
	bool bShowActorName = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show glowing mesh when selecting actors."))
	bool bShowHighlight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Select the object types you want to query when selecting actors"))
	TArray<TEnumAsByte<ECollisionChannel>> SelectCollisionTypesToQuery = {
		ECollisionChannel::ECC_WorldStatic,	 ECollisionChannel::ECC_WorldDynamic, ECollisionChannel::ECC_Pawn,
		ECollisionChannel::ECC_Destructible, ECollisionChannel::ECC_Vehicle,	  ECollisionChannel::ECC_PhysicsBody
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = StartUp, meta = (ToolTip = "Show the Spatial Debugger automatically at startup"))
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization,
			  meta = (ToolTip = "Show a transparent Worker Region cuboid representing the area of authority for each server worker"))
	bool bShowWorkerRegions = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization,
			  meta = (ToolTip = "Height at which the origin of each worker region cuboid is placed"))
	float WorkerRegionHeight = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Vertical scale to apply to each worker region cuboid"))
	float WorkerRegionVerticalScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Opacity of the worker region cuboids"))
	float WorkerRegionOpacity = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Auth Icon"))
	UTexture2D* AuthTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Auth Intent Icon"))
	UTexture2D* AuthIntentTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Unlocked Icon"))
	UTexture2D* UnlockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Locked Icon"))
	UTexture2D* LockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Box Icon"))
	UTexture2D* BoxTexture;

	// This will be drawn instead of the mouse cursor when selecting an actor
	UPROPERTY(EditDefaultsOnly, Category = Visualization, meta = (ToolTip = "Texture to use when selecting an actor"))
	UTexture2D* CrosshairTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "WorldSpace offset of tag from actor pivot"))
	FVector WorldSpaceActorTagOffset = FVector(0.0f, 0.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Color used for any server with an unresolved name"))
	FColor InvalidServerTintColor = FColor::Magenta;

	UPROPERTY(ReplicatedUsing = OnRep_SetWorkerRegions)
	TArray<FWorkerRegionInfo> WorkerRegions;

	UFUNCTION()
	virtual void OnRep_SetWorkerRegions();

	UFUNCTION()
	void OnToggleConfigUI();

	UFUNCTION(BlueprintCallable, Category = Visualization)
	void ToggleSelectActor();

	UFUNCTION()
	void OnSelectActor();

	UFUNCTION()
	void OnHighlightActor();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Visualization)
	bool IsSelectActorEnabled() const;

private:
	UFUNCTION()
	void DefaultOnConfigUIClosed();

public:
	UFUNCTION(BlueprintCallable, Category = Visualization)
	void SetShowWorkerRegions(const bool bNewShow);

	void ActorAuthorityChanged(const Worker_ComponentSetAuthorityChangeOp& AuthOp) const;
	void ActorAuthorityIntentChanged(Worker_EntityId EntityId, VirtualWorkerId NewIntentVirtualWorkerId) const;

#if WITH_EDITOR
	void EditorRefreshWorkerRegions();
	static void EditorRefreshDisplay();
	bool EditorAllowWorkerBoundaries() const;
	void EditorSpatialToggleDebugger(bool bEnabled);
#endif

private:
	void LoadIcons();

	// FOnEntityAdded/FOnEntityRemoved Delegates
	void OnEntityAdded(const Worker_EntityId EntityId);
	void OnEntityRemoved(const Worker_EntityId EntityId);

	// FDebugDrawDelegate
	void DrawDebug(UCanvas* Canvas, APlayerController* Controller);

	FVector GetLocalPawnLocation();

	// Allow user to select actor(s) for debugging - the mesh on the actor must have collision presets enabled to block on at least one of
	// the object channels
	void SelectActorsToTag(UCanvas* Canvas);

	void HighlightActorUnderCursor(TWeakObjectPtr<AActor>& NewHoverActor);

	TWeakObjectPtr<AActor> GetActorAtPosition(const FVector2D& MousePosition);

	TWeakObjectPtr<AActor> GetHitActor();

	FVector2D ProjectActorToScreen(const TWeakObjectPtr<AActor> Actor, const FVector& PlayerLocation);

	void RevertHoverMaterials();

	void DrawTag(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName,
				 const bool bCentre);
	void DrawDebugLocalPlayer(UCanvas* Canvas);

	void CreateWorkerRegions();
	void DestroyWorkerRegions();

	FColor GetTextColorForBackgroundColor(const FColor& BackgroundColor) const;
	int32 GetNumberOfDigitsIn(int32 SomeNumber) const;

#if WITH_EDITOR
	void EditorInitialiseWorkerRegions();
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif

	static const int ENTITY_ACTOR_MAP_RESERVATION_COUNT = 512;
	static const int PLAYER_TAG_VERTICAL_OFFSET = 18;

	enum EIcon
	{
		ICON_AUTH,
		ICON_AUTH_INTENT,
		ICON_UNLOCKED,
		ICON_LOCKED,
		ICON_BOX,
		ICON_MAX
	};

	USpatialNetDriver* NetDriver;

	// These mappings are maintained independently on each client
	// Mapping of the entities a client has checked out
	TMap<Worker_EntityId_Key, TWeakObjectPtr<AActor>> EntityActorMapping;

	FDelegateHandle DrawDebugDelegateHandle;
	FDelegateHandle OnEntityAddedHandle;
	FDelegateHandle OnEntityRemovedHandle;

	TWeakObjectPtr<APawn> LocalPawn;
	TWeakObjectPtr<APlayerController> LocalPlayerController;
	TWeakObjectPtr<APlayerState> LocalPlayerState;
	UFont* RenderFont;

	FFontRenderInfo FontRenderInfo;
	FCanvasIcon Icons[ICON_MAX];

	USpatialDebuggerConfigUI* ConfigUIWidget;

	// Mode for selecting actors under the cursor - should only be visible in the runtime config UI
	bool bSelectActor = false;

	// Actors selected by user for debugging
	TArray<TWeakObjectPtr<AActor>> SelectedActors;

	// Highlighted actor under the mouse cursor
	TWeakObjectPtr<AActor> HoverActor;
	// Highlighted actor original materials and components
	TArray<TWeakObjectPtr<UMaterialInterface>> ActorMeshMaterials;
	TArray<TWeakObjectPtr<UMeshComponent>> ActorMeshComponents;
	// Material for highlighting actor
	UPROPERTY()
	UMaterialInterface* WireFrameMaterial;

	// All actors under the mouse cursor
	TArray<TWeakObjectPtr<AActor>> HitActors;

	// Index for selecting the highlighted actor when multiple are under the mouse cursor
	int32 HoverIndex;

	// Mouse position to avoid unnecessary raytracing when mouse has not moved
	FVector2D MousePosition;

	// Select actor object types to query
	FCollisionObjectQueryParams CollisionObjectParams;
};
