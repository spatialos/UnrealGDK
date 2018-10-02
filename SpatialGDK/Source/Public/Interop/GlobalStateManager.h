// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Utils/SchemaUtils.h"

#include "EngineClasses/SpatialNetDriver.h" // TODO: Remove this.

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "GlobalStateManager.generated.h"

class USpatialNetDriver;
class USpatialActorChannel;
class USpatialStaticComponentView;
class USpatialSender;

DECLARE_LOG_CATEGORY_EXTERN(LogGlobalStateManager, Log, All)

UCLASS()
class SPATIALGDK_API UGlobalStateManager : public UObject
{
	GENERATED_BODY()

public:

	void Init(USpatialNetDriver* InNetDriver);

	void ApplyData(const Worker_ComponentData& Data);
	void ApplyUpdate(const Worker_ComponentUpdate& Update);

	void LinkExistingSingletonActors();
	void ExecuteInitialSingletonActorReplication();
	void UpdateSingletonEntityId(const FString& ClassName, const Worker_EntityId SingletonEntityId);

	bool IsSingletonEntity(Worker_EntityId EntityId);

	void FindDeploymentMapURL();
	void SetDeploymentMapURL(FString MapURL);
	void WorldWipe(const USpatialNetDriver::ServerTravelDelegate& Delegate);
	void DeleteEntities(const Worker_EntityQueryResponseOp& Op);
	void LoadSnapshot(const USpatialNetDriver::ServerTravelDelegate& Delegate);

	FString DeploymentMapURL;

private:
	void GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel);

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	USpatialStaticComponentView* StaticComponentView;

	UPROPERTY()
	USpatialSender* Sender;

	StringToEntityMap SingletonNameToEntityId;
};
