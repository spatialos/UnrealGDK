// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialPackageMapClient.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialPackageMapClient() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPackageMapClient_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPackageMapClient();
	ENGINE_API UClass* Z_Construct_UClass_UPackageMapClient();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialClassInfoManager_NoRegister();
// End Cross Module References
	void USpatialPackageMapClient::StaticRegisterNativesUSpatialPackageMapClient()
	{
	}
	UClass* Z_Construct_UClass_USpatialPackageMapClient_NoRegister()
	{
		return USpatialPackageMapClient::StaticClass();
	}
	struct Z_Construct_UClass_USpatialPackageMapClient_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ClassInfoManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ClassInfoManager;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialPackageMapClient_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UPackageMapClient,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialPackageMapClient_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialPackageMapClient.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialPackageMapClient.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialPackageMapClient.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialPackageMapClient, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_NetDriver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_ClassInfoManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialPackageMapClient.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_ClassInfoManager = { "ClassInfoManager", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialPackageMapClient, ClassInfoManager), Z_Construct_UClass_USpatialClassInfoManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_ClassInfoManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_ClassInfoManager_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialPackageMapClient_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_NetDriver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialPackageMapClient_Statics::NewProp_ClassInfoManager,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialPackageMapClient_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialPackageMapClient>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialPackageMapClient_Statics::ClassParams = {
		&USpatialPackageMapClient::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialPackageMapClient_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialPackageMapClient_Statics::PropPointers),
		0,
		0x001000A8u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialPackageMapClient_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialPackageMapClient_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialPackageMapClient()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialPackageMapClient_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialPackageMapClient, 3474838439);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialPackageMapClient>()
	{
		return USpatialPackageMapClient::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialPackageMapClient(Z_Construct_UClass_USpatialPackageMapClient, &USpatialPackageMapClient::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialPackageMapClient"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialPackageMapClient);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
