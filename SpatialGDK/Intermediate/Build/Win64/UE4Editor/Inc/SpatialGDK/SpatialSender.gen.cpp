// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/SpatialSender.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialSender() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialSender_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialSender();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorGroupManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialClassInfoManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPackageMapClient_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialReceiver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialWorkerConnection_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStaticComponentView_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
// End Cross Module References
	void USpatialSender::StaticRegisterNativesUSpatialSender()
	{
	}
	UClass* Z_Construct_UClass_USpatialSender_NoRegister()
	{
		return USpatialSender::StaticClass();
	}
	struct Z_Construct_UClass_USpatialSender_Statics
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
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ClassInfoManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ClassInfoManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PackageMap_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_PackageMap;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Receiver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Receiver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Connection_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Connection;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_StaticComponentView_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_StaticComponentView;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialSender_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Interop/SpatialSender.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_ActorGroupManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_ActorGroupManager = { "ActorGroupManager", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, ActorGroupManager), Z_Construct_UClass_UActorGroupManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_ActorGroupManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_ActorGroupManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_ClassInfoManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_ClassInfoManager = { "ClassInfoManager", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, ClassInfoManager), Z_Construct_UClass_USpatialClassInfoManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_ClassInfoManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_ClassInfoManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_PackageMap_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_PackageMap = { "PackageMap", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, PackageMap), Z_Construct_UClass_USpatialPackageMapClient_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_PackageMap_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_PackageMap_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_Receiver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_Receiver = { "Receiver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, Receiver), Z_Construct_UClass_USpatialReceiver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_Receiver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_Receiver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_Connection_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_Connection = { "Connection", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, Connection), Z_Construct_UClass_USpatialWorkerConnection_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_Connection_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_Connection_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_StaticComponentView_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_StaticComponentView = { "StaticComponentView", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, StaticComponentView), Z_Construct_UClass_USpatialStaticComponentView_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_StaticComponentView_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_StaticComponentView_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialSender_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialSender.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialSender_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialSender, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::NewProp_NetDriver_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialSender_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_ActorGroupManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_ClassInfoManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_PackageMap,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_Receiver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_Connection,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_StaticComponentView,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialSender_Statics::NewProp_NetDriver,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialSender_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialSender>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialSender_Statics::ClassParams = {
		&USpatialSender::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialSender_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::PropPointers),
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialSender_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialSender_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialSender()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialSender_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialSender, 3061270598);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialSender>()
	{
		return USpatialSender::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialSender(Z_Construct_UClass_USpatialSender, &USpatialSender::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialSender"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialSender);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
