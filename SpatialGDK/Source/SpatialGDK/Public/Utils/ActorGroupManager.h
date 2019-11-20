// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialConstants.h"

#include "ActorGroupManager.generated.h"

USTRUCT()
struct FWorkerType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	FName WorkerTypeName;

	FWorkerType() : WorkerTypeName(NAME_None)
	{
	}

	FWorkerType(FName InWorkerTypeName) : WorkerTypeName(InWorkerTypeName)
	{
	}
};

USTRUCT()
struct FActorGroupInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName Name;

	/** The server worker type that has authority of all classes in this actor group. */
	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	FWorkerType OwningWorkerType;

	// Using TSoftClassPtr here to prevent eagerly loading all classes.
	/** The Actor classes contained within this group. Children of these classes will also be included. */	
	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	TSet<TSoftClassPtr<AActor>> ActorClasses;
	
	FActorGroupInfo() : Name(NAME_None), OwningWorkerType()
	{
	}
};

UCLASS(Config=SpatialGDKSettings)
class SPATIALGDK_API UActorGroupManager : public UObject
{
	GENERATED_BODY()

private:
	TMap<TSoftClassPtr<AActor>, FName> ClassPathToActorGroup;

	TMap<FName, FName> ActorGroupToWorkerType;

	FName DefaultWorkerType;

public:
	void Init();

	// Returns the first ActorGroup that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetActorGroupForClass(TSubclassOf<AActor> Class);

	// Returns the Server worker type that is authoritative over the ActorGroup
	// that contains this class (or parent class). Returns DefaultWorkerType
	// if no mapping is found.
	FName GetWorkerTypeForClass(TSubclassOf<AActor> Class);

	// Returns the Server worker type that is authoritative over this ActorGroup.
	FName GetWorkerTypeForActorGroup(const FName& ActorGroup) const;

	// Returns true if ActorA and ActorB are contained in ActorGroups that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB);
};
