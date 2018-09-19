// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Utils/SchemaUtils.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include "GlobalStateManager.generated.h"

class USpatialNetDriver;
class USpatialActorChannel;
class USpatialView;
class USpatialSender;

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

private:
	void GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel);

private:
	USpatialNetDriver* NetDriver;
	USpatialView* View;
	USpatialSender* Sender;

	StringToEntityMap SingletonNameToEntityId;
	StringToEntityMap StablyNamedPathToEntityId;

};
