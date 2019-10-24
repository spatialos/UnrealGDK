// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/SnapshotManager.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSnapshotManager() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USnapshotManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USnapshotManager();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialReceiver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UGlobalStateManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
// End Cross Module References
	void USnapshotManager::StaticRegisterNativesUSnapshotManager()
	{
	}
	UClass* Z_Construct_UClass_USnapshotManager_NoRegister()
	{
		return USnapshotManager::StaticClass();
	}
	struct Z_Construct_UClass_USnapshotManager_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Receiver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Receiver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GlobalStateManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_GlobalStateManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USnapshotManager_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USnapshotManager_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Interop/SnapshotManager.h" },
		{ "ModuleRelativePath", "Public/Interop/SnapshotManager.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USnapshotManager_Statics::NewProp_Receiver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SnapshotManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USnapshotManager_Statics::NewProp_Receiver = { "Receiver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USnapshotManager, Receiver), Z_Construct_UClass_USpatialReceiver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USnapshotManager_Statics::NewProp_Receiver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USnapshotManager_Statics::NewProp_Receiver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USnapshotManager_Statics::NewProp_GlobalStateManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SnapshotManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USnapshotManager_Statics::NewProp_GlobalStateManager = { "GlobalStateManager", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USnapshotManager, GlobalStateManager), Z_Construct_UClass_UGlobalStateManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USnapshotManager_Statics::NewProp_GlobalStateManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USnapshotManager_Statics::NewProp_GlobalStateManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USnapshotManager_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SnapshotManager.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USnapshotManager_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USnapshotManager, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USnapshotManager_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USnapshotManager_Statics::NewProp_NetDriver_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USnapshotManager_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USnapshotManager_Statics::NewProp_Receiver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USnapshotManager_Statics::NewProp_GlobalStateManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USnapshotManager_Statics::NewProp_NetDriver,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USnapshotManager_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USnapshotManager>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USnapshotManager_Statics::ClassParams = {
		&USnapshotManager::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USnapshotManager_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USnapshotManager_Statics::PropPointers),
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USnapshotManager_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USnapshotManager_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USnapshotManager()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USnapshotManager_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USnapshotManager, 1985686713);
	template<> SPATIALGDK_API UClass* StaticClass<USnapshotManager>()
	{
		return USnapshotManager::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USnapshotManager(Z_Construct_UClass_USnapshotManager, &USnapshotManager::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USnapshotManager"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USnapshotManager);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
