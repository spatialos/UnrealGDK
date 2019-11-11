// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaDatabase.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialClassInfoManager.generated.h"

FORCEINLINE void ForAllSchemaComponentTypes(TFunction<void(ESchemaComponentType)> Callback)
{
	for (int32 Type = SCHEMA_Begin; Type < SCHEMA_Count; Type++)
	{
		Callback(ESchemaComponentType(Type));
	}
}

FORCEINLINE ESchemaComponentType GetGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return SCHEMA_OwnerOnly;
	default:
		return SCHEMA_Data;
	}
}

struct FRPCInfo
{
	ESchemaComponentType Type;
	uint32 Index;
};

struct FHandoverPropertyInfo
{
	uint16 Handle;
	int32 Offset;
	int32 ArrayIdx;
	UProperty* Property;
};

struct FInterestPropertyInfo
{
	UProperty* Property;
	int32 Offset;
};

USTRUCT()
struct FClassInfo
{
	GENERATED_BODY()

	TWeakObjectPtr<UClass> Class;

	// Exists for all classes
	TArray<UFunction*> RPCs;
	TMap<UFunction*, FRPCInfo> RPCInfoMap;
	TArray<FHandoverPropertyInfo> HandoverProperties;
	TArray<FInterestPropertyInfo> InterestProperties;

	// For Actors and default Subobjects belonging to Actors
	Worker_ComponentId SchemaComponents[ESchemaComponentType::SCHEMA_Count] = {};

	// Only for Actors
	TMap<uint32, TSharedRef<const FClassInfo>> SubobjectInfo;

	// Only for default Subobjects belonging to Actors
	FName SubobjectName;

	// Only for Subobject classes
	TArray<TSharedRef<const FClassInfo>> DynamicSubobjectInfo;

	FName ActorGroup;
	FName WorkerType;
};

class UActorGroupManager;
class USpatialNetDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialClassInfoManager, Log, All)

UCLASS()
class SPATIALGDK_API USpatialClassInfoManager : public UObject
{
	GENERATED_BODY()

public:

	bool TryInit(USpatialNetDriver* NetDriver, UActorGroupManager* ActorGroupManager);

	// Checks whether a class is supported and quits the game if not. This is to avoid crashing
	// when running with an out-of-date schema database.
	bool ValidateOrExit_IsSupportedClass(const FString& PathName);

	// Returns true if the class path corresponds to an Actor or Subobject class path in SchemaDatabase
	// In PIE, PathName must be NetworkRemapped (bReading = false)
	bool IsSupportedClass(const FString& PathName) const;

	const FClassInfo& GetOrCreateClassInfoByClass(UClass* Class);
	const FClassInfo& GetOrCreateClassInfoByObject(UObject* Object);
	const FClassInfo& GetClassInfoByComponentId(Worker_ComponentId ComponentId);

	UClass* GetClassByComponentId(Worker_ComponentId ComponentId);
	bool GetOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset);
	ESchemaComponentType GetCategoryByComponentId(Worker_ComponentId ComponentId);

	Worker_ComponentId GetComponentIdForClass(const UClass& Class) const;
	TArray<Worker_ComponentId> GetComponentIdsForClassHierarchy(const UClass& BaseClass, const bool bIncludeDerivedTypes = true) const;
	
	const FRPCInfo& GetRPCInfo(UObject* Object, UFunction* Function);

	uint32 GetComponentIdFromLevelPath(const FString& LevelPath);
	bool IsSublevelComponent(Worker_ComponentId ComponentId);

	UPROPERTY()
	USchemaDatabase* SchemaDatabase;

private:
	void CreateClassInfoForClass(UClass* Class);
	void TryCreateClassInfoForComponentId(Worker_ComponentId ComponentId);

	void FinishConstructingActorClassInfo(const FString& ClassPath, TSharedRef<FClassInfo>& Info);
	void FinishConstructingSubobjectClassInfo(const FString& ClassPath, TSharedRef<FClassInfo>& Info);

	void QuitGame();

private:
	UPROPERTY()
	USpatialNetDriver* NetDriver;

	UPROPERTY()
	UActorGroupManager* ActorGroupManager;

	TMap<TWeakObjectPtr<UClass>, TSharedRef<FClassInfo>> ClassInfoMap;
	TMap<Worker_ComponentId, TSharedRef<FClassInfo>> ComponentToClassInfoMap;
	TMap<Worker_ComponentId, uint32> ComponentToOffsetMap;
	TMap<Worker_ComponentId, ESchemaComponentType> ComponentToCategoryMap;
};
