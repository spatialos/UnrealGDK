// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/SpatialClassInfoManager.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialClassInfoManager() {}
// Cross Module References
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FClassInfo();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialClassInfoManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialClassInfoManager();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorGroupManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USchemaDatabase_NoRegister();
// End Cross Module References
class UScriptStruct* FClassInfo::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FClassInfo_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FClassInfo, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("ClassInfo"), sizeof(FClassInfo), Get_Z_Construct_UScriptStruct_FClassInfo_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FClassInfo>()
{
	return FClassInfo::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FClassInfo(FClassInfo::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("ClassInfo"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFClassInfo
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFClassInfo()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("ClassInfo")),new UScriptStruct::TCppStructOps<FClassInfo>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFClassInfo;
	struct Z_Construct_UScriptStruct_FClassInfo_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FClassInfo_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialClassInfoManager.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FClassInfo_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FClassInfo>();
	}
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FClassInfo_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"ClassInfo",
		sizeof(FClassInfo),
		alignof(FClassInfo),
		nullptr,
		0,
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FClassInfo_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FClassInfo_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FClassInfo()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FClassInfo_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("ClassInfo"), sizeof(FClassInfo), Get_Z_Construct_UScriptStruct_FClassInfo_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FClassInfo_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FClassInfo_Hash() { return 1377731658U; }
	void USpatialClassInfoManager::StaticRegisterNativesUSpatialClassInfoManager()
	{
	}
	UClass* Z_Construct_UClass_USpatialClassInfoManager_NoRegister()
	{
		return USpatialClassInfoManager::StaticClass();
	}
	struct Z_Construct_UClass_USpatialClassInfoManager_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorGroupManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ActorGroupManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SchemaDatabase_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SchemaDatabase;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialClassInfoManager_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialClassInfoManager_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Interop/SpatialClassInfoManager.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialClassInfoManager.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_ActorGroupManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialClassInfoManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_ActorGroupManager = { "ActorGroupManager", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialClassInfoManager, ActorGroupManager), Z_Construct_UClass_UActorGroupManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_ActorGroupManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_ActorGroupManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialClassInfoManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialClassInfoManager, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_NetDriver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_SchemaDatabase_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialClassInfoManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_SchemaDatabase = { "SchemaDatabase", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialClassInfoManager, SchemaDatabase), Z_Construct_UClass_USchemaDatabase_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_SchemaDatabase_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_SchemaDatabase_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialClassInfoManager_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_ActorGroupManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_NetDriver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialClassInfoManager_Statics::NewProp_SchemaDatabase,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialClassInfoManager_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialClassInfoManager>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialClassInfoManager_Statics::ClassParams = {
		&USpatialClassInfoManager::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialClassInfoManager_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialClassInfoManager_Statics::PropPointers),
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialClassInfoManager_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialClassInfoManager_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialClassInfoManager()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialClassInfoManager_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialClassInfoManager, 626512984);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialClassInfoManager>()
	{
		return USpatialClassInfoManager::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialClassInfoManager(Z_Construct_UClass_USpatialClassInfoManager, &USpatialClassInfoManager::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialClassInfoManager"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialClassInfoManager);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
