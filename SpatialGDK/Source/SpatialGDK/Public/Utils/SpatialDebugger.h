// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Info.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialDebugger.generated.h"

class APawn;
class APlayerController;
class APlayerState;
class USpatialNetDriver;
class UFont;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialDebugger, Log, All);

DECLARE_STATS_GROUP(TEXT("SpatialDebugger"), STATGROUP_SpatialDebugger, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("DrawDebug"), STAT_DrawDebug, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawTag"), STAT_DrawTag, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("Projection"), STAT_Projection, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawIcons"), STAT_DrawIcons, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawText"), STAT_DrawText, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("BuildText"), STAT_BuildText, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("SortingActors"), STAT_SortingActors, STATGROUP_SpatialDebugger);

UCLASS(SpatialType=Singleton, Blueprintable)
class SPATIALGDK_API ASpatialDebugger :
	public AInfo
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(Exec, Category = "SpatialGDK", BlueprintCallable)
	void SpatialToggleDebugger();

	// TODO: These should all be exposed through a runtime UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "X location of player data panel"))
	int PlayerPanelStartX = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LocalPlayer, meta = (ToolTip = "Y location of player data panel"))
	int PlayerPanelStartY = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General, meta = (ToolTip = "Maximum range from local player that tags will be drawn out to"))
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show server authority for every entity in range"))
	bool bShowAuth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show authority intent for every entity in range"))
	bool bShowAuthIntent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show lock status for every entity in range"))
	bool bShowLock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show EntityId for every entity in range"))
	bool bShowEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Visualization, meta = (ToolTip = "Show Actor Name for every entity in range"))
	bool bShowActorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StartUp, meta = (ToolTip = "Show the Spatial Debugger automatically at startup"))
	bool bAutoStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Auth Icon"))
	UTexture2D *AuthTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Auth Intent Icon"))
	UTexture2D *AuthIntentTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Unlocked Icon"))
	UTexture2D *UnlockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "Texture to use for the Locked Icon"))
	UTexture2D *LockedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Visualization, meta = (ToolTip = "WorldSpace offset of tag from actor pivot"))
	FVector WorldSpaceActorTagOffset;

private:

	void LoadIcons();

	void OnEntityAdded(const Worker_EntityId EntityId);
	void OnEntityRemoved(const Worker_EntityId EntityId);

	void DrawDebug(UCanvas* Canvas, APlayerController* Controller);
	void DrawTag(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName);
	void DrawDebugLocalPlayer(UCanvas* Canvas);

	int32 GetVirtualWorkerId(const Worker_EntityId EntityId) const;

	static int32 HashPosition(const FVector& Position);

	static const int ENTITY_ACTOR_MAP_RESERVATION_COUNT = 512;
	static const int STACKED_TAG_VERTICAL_OFFSET = 18;
	static const int VIRTUAL_WORKER_MAX_COUNT = 4;

	enum EIcon
	{
		ICON_AUTH,
		ICON_AUTH_INTENT,
		ICON_UNLOCKED,
		ICON_LOCKED,
		ICON_MAX
	};

	USpatialNetDriver* NetDriver;

	// These mappings are maintained independently on each client
	// Mapping of the entities a client has checked out
	TMap<int64, TWeakObjectPtr<AActor>> EntityActorMapping;

	FDelegateHandle DrawDebugDelegateHandle;
	FDelegateHandle OnEntityAddedHandle;
	FDelegateHandle OnEntityRemovedHandle;

	TWeakObjectPtr<APawn> LocalPawn;
	TWeakObjectPtr<APlayerController> LocalPlayerController;
	TWeakObjectPtr<APlayerState> LocalPlayerState;
	UFont* RenderFont;

	FFontRenderInfo FontRenderInfo;
	FCanvasIcon Icons[ICON_MAX];

	bool bActorSortRequired;

	const FColor WorkerColors[VIRTUAL_WORKER_MAX_COUNT] =
	{
		FColor::Blue,
		FColor::Green,
		FColor::Yellow,
		FColor::Orange
	};
};
