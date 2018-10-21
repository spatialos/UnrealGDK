// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Utils/SchemaDatabase.h"

#include <improbable/c_worker.h>

#include "SpatialTypebindingManager.generated.h"

enum EComponentType : int32
{
	TYPE_Invalid = -1,

	// Properties
	TYPE_Data,
	TYPE_OwnerOnly,
	TYPE_Handover,

	// RPCs
	TYPE_ClientRPC,
	TYPE_ServerRPC,
	TYPE_NetMulticastRPC,
	TYPE_CrossServerRPC,

	TYPE_Count
};

FORCEINLINE void ForAllSchemaComponentTypes(TFunction<void(EComponentType)> Callback)
{
	for (int32 Type = TYPE_Data; Type < TYPE_Count; Type++)
	{
		Callback(EComponentType(Type));
	}
}

FORCEINLINE EComponentType GetGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return TYPE_OwnerOnly;
	default:
		return TYPE_Data;
	}
}

struct FRPCInfo
{
	EComponentType Type;
	uint32 Index;
};

struct FHandoverPropertyInfo
{
	uint16 Handle;
	int32 Offset;
	int32 ArrayIdx;
	UProperty* Property;
};

USTRUCT()
struct FClassInfo
{
	GENERATED_BODY()

	UClass* Class;

	TMap<EComponentType, TArray<UFunction*>> RPCs;
	TMap<UFunction*, FRPCInfo> RPCInfoMap;

	TArray<FHandoverPropertyInfo> HandoverProperties;

	Worker_ComponentId SchemaComponents[EComponentType::TYPE_Count];

	TMap<uint32, TSharedPtr<FClassInfo>> SubobjectInfo;
};

UCLASS()
class SPATIALGDK_API USpatialTypebindingManager : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	bool IsSupportedClass(UClass* Class);
	FClassInfo* FindClassInfoByClass(UClass* Class);
	FClassInfo* FindClassInfoByClassAndOffset(UClass* Class, uint32 Offset);
	FClassInfo* FindClassInfoByComponentId(Worker_ComponentId ComponentId);
	UClass* FindClassByComponentId(Worker_ComponentId ComponentId);

	TArray<UObject*> GetHandoverSubobjects(AActor* Actor);

	bool FindOffsetByComponentId(Worker_ComponentId ComponentId, uint32& OutOffset);
	EComponentType FindCategoryByComponentId(Worker_ComponentId ComponentId);

private:
	void FindSupportedClasses();
	void CreateTypebindings();

private:
	UPROPERTY()
	USchemaDatabase* SchemaDatabase;

	UPROPERTY()
	TArray<UClass*> SupportedClasses;

	TMap<UClass*, TSharedPtr<FClassInfo>> ClassInfoMap;

	TMap<Worker_ComponentId, UClass*> ComponentToClassMap;
	TMap<Worker_ComponentId, uint32> ComponentToOffsetMap;
	TMap<Worker_ComponentId, EComponentType> ComponentToCategoryMap;
};
