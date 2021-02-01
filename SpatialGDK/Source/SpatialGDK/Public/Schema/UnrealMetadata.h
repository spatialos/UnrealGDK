// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Component.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

#include "GameFramework/Actor.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DEFINE_LOG_CATEGORY_STATIC(LogSpatialUnrealMetadata, Warning, All);

using SubobjectToOffsetMap = TMap<UObject*, uint32>;

namespace SpatialGDK
{
struct UnrealMetadata : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::UNREAL_METADATA_COMPONENT_ID;

	UnrealMetadata() = default;

	UnrealMetadata(const TSchemaOption<FUnrealObjectRef>& InStablyNamedRef, const FString& InClassPath,
				   const TSchemaOption<bool>& InbNetStartup)
		: StablyNamedRef(InStablyNamedRef)
		, ClassPath(InClassPath)
		, bNetStartup(InbNetStartup)
	{
	}

	UnrealMetadata(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (Schema_GetObjectCount(ComponentObject, SpatialConstants::UNREAL_METADATA_STABLY_NAMED_REF_ID) == 1)
		{
			StablyNamedRef = GetObjectRefFromSchema(ComponentObject, SpatialConstants::UNREAL_METADATA_STABLY_NAMED_REF_ID);
		}
		ClassPath = GetStringFromSchema(ComponentObject, SpatialConstants::UNREAL_METADATA_CLASS_PATH_ID);

		if (Schema_GetBoolCount(ComponentObject, SpatialConstants::UNREAL_METADATA_NET_STARTUP_ID) == 1)
		{
			bNetStartup = GetBoolFromSchema(ComponentObject, SpatialConstants::UNREAL_METADATA_NET_STARTUP_ID);
		}
	}

	Worker_ComponentData CreateUnrealMetadataData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (StablyNamedRef.IsSet())
		{
			AddObjectRefToSchema(ComponentObject, SpatialConstants::UNREAL_METADATA_STABLY_NAMED_REF_ID, StablyNamedRef.GetValue());
		}
		AddStringToSchema(ComponentObject, SpatialConstants::UNREAL_METADATA_CLASS_PATH_ID, ClassPath);
		if (bNetStartup.IsSet())
		{
			Schema_AddBool(ComponentObject, SpatialConstants::UNREAL_METADATA_NET_STARTUP_ID, bNetStartup.GetValue());
		}

		return Data;
	}

	FORCEINLINE UClass* GetNativeEntityClass()
	{
		if (NativeClass.IsValid())
		{
			return NativeClass.Get();
		}

#if !UE_BUILD_SHIPPING
		if (NativeClass.IsStale())
		{
			UE_LOG(LogSpatialUnrealMetadata, Warning, TEXT("UnrealMetadata native class %s unloaded whilst entity in view."), *ClassPath);
		}
#endif
		UClass* Class = FindObject<UClass>(nullptr, *ClassPath, false);

		// Unfortunately StablyNameRef doesn't mean NameStableForNetworking as we add a StablyNameRef for every startup actor (see
		// USpatialSender::CreateEntity)
		// TODO: UNR-2537 Investigate why FindObject can be used the first time the actor comes into view for a client but not subsequent
		// loads.
		if (Class == nullptr && !(StablyNamedRef.IsSet() && bNetStartup.IsSet() && bNetStartup.GetValue()))
		{
			if (GetDefault<USpatialGDKSettings>()->bAsyncLoadNewClassesOnEntityCheckout)
			{
				UE_LOG(LogSpatialUnrealMetadata, Warning,
					   TEXT("Class couldn't be found even though async loading on entity checkout is enabled. Will attempt to load it "
							"synchronously. Class: %s"),
					   *ClassPath);
			}

			Class = LoadObject<UClass>(nullptr, *ClassPath);
		}

		if (Class != nullptr && Class->IsChildOf<AActor>())
		{
			NativeClass = Class;
			return Class;
		}

		return nullptr;
	}

	TSchemaOption<FUnrealObjectRef> StablyNamedRef;
	FString ClassPath;
	TSchemaOption<bool> bNetStartup;

	TWeakObjectPtr<UClass> NativeClass;
};

FORCEINLINE SubobjectToOffsetMap CreateOffsetMapFromActor(AActor* Actor, const FClassInfo& Info)
{
	SubobjectToOffsetMap SubobjectNameToOffset;

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		UObject* Subobject = StaticFindObjectFast(UObject::StaticClass(), Actor, SubobjectInfoPair.Value->SubobjectName);
		uint32 Offset = SubobjectInfoPair.Key;

		if (Subobject != nullptr && Subobject->IsPendingKill() == false && Subobject->IsSupportedForNetworking())
		{
			SubobjectNameToOffset.Add(Subobject, Offset);
		}
	}

	return SubobjectNameToOffset;
}

} // namespace SpatialGDK
