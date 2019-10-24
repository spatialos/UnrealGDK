// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Utils/ActorGroupManager.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeActorGroupManager() {}
// Cross Module References
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FActorGroupInfo();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FWorkerType();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorGroupManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorGroupManager();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
// End Cross Module References
class UScriptStruct* FActorGroupInfo::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FActorGroupInfo_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FActorGroupInfo, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("ActorGroupInfo"), sizeof(FActorGroupInfo), Get_Z_Construct_UScriptStruct_FActorGroupInfo_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FActorGroupInfo>()
{
	return FActorGroupInfo::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FActorGroupInfo(FActorGroupInfo::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("ActorGroupInfo"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFActorGroupInfo
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFActorGroupInfo()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("ActorGroupInfo")),new UScriptStruct::TCppStructOps<FActorGroupInfo>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFActorGroupInfo;
	struct Z_Construct_UScriptStruct_FActorGroupInfo_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClasses_MetaData[];
#endif
		static const UE4CodeGen_Private::FSetPropertyParams NewProp_ActorClasses;
		static const UE4CodeGen_Private::FSoftClassPropertyParams NewProp_ActorClasses_ElementProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_OwningWorkerType_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_OwningWorkerType;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Name_MetaData[];
#endif
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_Name;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorGroupInfo_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FActorGroupInfo>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
		{ "ToolTip", "The Actor classes contained within this group. Children of these classes will also be included." },
	};
#endif
	const UE4CodeGen_Private::FSetPropertyParams Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses = { "ActorClasses", nullptr, (EPropertyFlags)0x0014000000000001, UE4CodeGen_Private::EPropertyGenFlags::Set, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorGroupInfo, ActorClasses), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses_MetaData)) };
	const UE4CodeGen_Private::FSoftClassPropertyParams Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses_ElementProp = { "ActorClasses", nullptr, (EPropertyFlags)0x0004000000000001, UE4CodeGen_Private::EPropertyGenFlags::SoftClass, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_OwningWorkerType_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
		{ "ToolTip", "The server worker type that has authority of all classes in this actor group." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_OwningWorkerType = { "OwningWorkerType", nullptr, (EPropertyFlags)0x0010000000000001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorGroupInfo, OwningWorkerType), Z_Construct_UScriptStruct_FWorkerType, METADATA_PARAMS(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_OwningWorkerType_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_OwningWorkerType_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_Name_MetaData[] = {
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
	};
#endif
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_Name = { "Name", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorGroupInfo, Name), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_Name_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_Name_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FActorGroupInfo_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_ActorClasses_ElementProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_OwningWorkerType,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorGroupInfo_Statics::NewProp_Name,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FActorGroupInfo_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"ActorGroupInfo",
		sizeof(FActorGroupInfo),
		alignof(FActorGroupInfo),
		Z_Construct_UScriptStruct_FActorGroupInfo_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorGroupInfo_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FActorGroupInfo()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FActorGroupInfo_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("ActorGroupInfo"), sizeof(FActorGroupInfo), Get_Z_Construct_UScriptStruct_FActorGroupInfo_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FActorGroupInfo_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FActorGroupInfo_Hash() { return 3419038062U; }
class UScriptStruct* FWorkerType::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FWorkerType_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FWorkerType, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("WorkerType"), sizeof(FWorkerType), Get_Z_Construct_UScriptStruct_FWorkerType_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FWorkerType>()
{
	return FWorkerType::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FWorkerType(FWorkerType::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("WorkerType"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFWorkerType
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFWorkerType()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("WorkerType")),new UScriptStruct::TCppStructOps<FWorkerType>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFWorkerType;
	struct Z_Construct_UScriptStruct_FWorkerType_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WorkerTypeName_MetaData[];
#endif
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_WorkerTypeName;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerType_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FWorkerType_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FWorkerType>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerType_Statics::NewProp_WorkerTypeName_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
	};
#endif
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UScriptStruct_FWorkerType_Statics::NewProp_WorkerTypeName = { "WorkerTypeName", nullptr, (EPropertyFlags)0x0010000000000001, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerType, WorkerTypeName), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerType_Statics::NewProp_WorkerTypeName_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerType_Statics::NewProp_WorkerTypeName_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FWorkerType_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerType_Statics::NewProp_WorkerTypeName,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FWorkerType_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"WorkerType",
		sizeof(FWorkerType),
		alignof(FWorkerType),
		Z_Construct_UScriptStruct_FWorkerType_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerType_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerType_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerType_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FWorkerType()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FWorkerType_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("WorkerType"), sizeof(FWorkerType), Get_Z_Construct_UScriptStruct_FWorkerType_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FWorkerType_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FWorkerType_Hash() { return 3523206314U; }
	void UActorGroupManager::StaticRegisterNativesUActorGroupManager()
	{
	}
	UClass* Z_Construct_UClass_UActorGroupManager_NoRegister()
	{
		return UActorGroupManager::StaticClass();
	}
	struct Z_Construct_UClass_UActorGroupManager_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UActorGroupManager_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorGroupManager_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Utils/ActorGroupManager.h" },
		{ "ModuleRelativePath", "Public/Utils/ActorGroupManager.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UActorGroupManager_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UActorGroupManager>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UActorGroupManager_Statics::ClassParams = {
		&UActorGroupManager::StaticClass,
		"SpatialGDKSettings",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UActorGroupManager_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UActorGroupManager_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UActorGroupManager()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UActorGroupManager_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UActorGroupManager, 1853370190);
	template<> SPATIALGDK_API UClass* StaticClass<UActorGroupManager>()
	{
		return UActorGroupManager::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UActorGroupManager(Z_Construct_UClass_UActorGroupManager, &UActorGroupManager::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UActorGroupManager"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UActorGroupManager);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
