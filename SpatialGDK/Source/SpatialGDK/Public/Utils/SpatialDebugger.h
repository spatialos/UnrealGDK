// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/WorkerRegion.h"
#include "SpatialCommonTypes.h"

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
};

UCLASS(SpatialType = (NotPersistent), Blueprintable, NotPlaceable)
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

	// TODO: Expose these through a runtime UI: https://improbableio.atlassian.net/browse/UNR-2359.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "X location of player data panel"))
	int PlayerPanelStartX = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "Y location of player data panel"))
	int PlayerPanelStartY = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General,
			  meta = (ToolTip = "Maximum range from local player that tags will be drawn out to"))
	float MaxRange = 100.0f * 100.0f; // 100m

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = StartUp, meta = (ToolTip = "Show the Spatial Debugger automatically at startup"))
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization,
			  meta = (ToolTip = "Show a transparent Worker Region cuboid representing the area of authority for each server worker"))
	bool bShowWorkerRegions = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "WorldSpace offset of tag from actor pivot"))
	FVector WorldSpaceActorTagOffset = FVector(0.0f, 0.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Color used for any server with an unresolved name"))
	FColor InvalidServerTintColor = FColor::Magenta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization,
			  meta = (ToolTip = "Vertical scale to apply to each worker region cuboid"))
	float WorkerRegionVerticalScale = 1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_SetWorkerRegions)
	TArray<FWorkerRegionInfo> WorkerRegions;

	UFUNCTION()
	virtual void OnRep_SetWorkerRegions();

	void ActorAuthorityChanged(const Worker_AuthorityChangeOp& AuthOp) const;
	void ActorAuthorityIntentChanged(Worker_EntityId EntityId, VirtualWorkerId NewIntentVirtualWorkerId) const;

private:
	void LoadIcons();

	// FOnEntityAdded/FOnEntityRemoved Delegates
	void OnEntityAdded(const Worker_EntityId EntityId);
	void OnEntityRemoved(const Worker_EntityId EntityId);

	// FDebugDrawDelegate
	void DrawDebug(UCanvas* Canvas, APlayerController* Controller);

	void DrawTag(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName);
	void DrawDebugLocalPlayer(UCanvas* Canvas);

	void CreateWorkerRegions();
	void DestroyWorkerRegions();

	FColor GetTextColorForBackgroundColor(const FColor& BackgroundColor) const;
	int32 GetNumberOfDigitsIn(int32 SomeNumber) const;

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
};
