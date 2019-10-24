// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialNetDriver() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver();
	ONLINESUBSYSTEMUTILS_API UClass* Z_Construct_UClass_UIpNetDriver();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld();
	ENGINE_API UClass* Z_Construct_UClass_UWorld_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_ULevel_NoRegister();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialGameInstance_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_ASpatialMetricsDisplay_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialMetrics_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UEntityPool_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USnapshotManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStaticComponentView_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPackageMapClient_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPlayerSpawner_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UGlobalStateManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialClassInfoManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorGroupManager_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialReceiver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialSender_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialDispatcher_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialWorkerConnection_NoRegister();
// End Cross Module References
	void USpatialNetDriver::StaticRegisterNativesUSpatialNetDriver()
	{
		UClass* Class = USpatialNetDriver::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "OnLevelAddedToWorld", &USpatialNetDriver::execOnLevelAddedToWorld },
			{ "OnMapLoaded", &USpatialNetDriver::execOnMapLoaded },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics
	{
		struct SpatialNetDriver_eventOnLevelAddedToWorld_Parms
		{
			ULevel* LoadedLevel;
			UWorld* OwningWorld;
		};
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_OwningWorld;
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_LoadedLevel;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::NewProp_OwningWorld = { "OwningWorld", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialNetDriver_eventOnLevelAddedToWorld_Parms, OwningWorld), Z_Construct_UClass_UWorld_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::NewProp_LoadedLevel = { "LoadedLevel", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialNetDriver_eventOnLevelAddedToWorld_Parms, LoadedLevel), Z_Construct_UClass_ULevel_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::NewProp_OwningWorld,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::NewProp_LoadedLevel,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialNetDriver, nullptr, "OnLevelAddedToWorld", sizeof(SpatialNetDriver_eventOnLevelAddedToWorld_Parms), Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics
	{
		struct SpatialNetDriver_eventOnMapLoaded_Parms
		{
			UWorld* LoadedWorld;
		};
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_LoadedWorld;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::NewProp_LoadedWorld = { "LoadedWorld", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialNetDriver_eventOnMapLoaded_Parms, LoadedWorld), Z_Construct_UClass_UWorld_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::NewProp_LoadedWorld,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialNetDriver, nullptr, "OnMapLoaded", sizeof(SpatialNetDriver_eventOnMapLoaded_Parms), Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00040401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister()
	{
		return USpatialNetDriver::StaticClass();
	}
	struct Z_Construct_UClass_USpatialNetDriver_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GameInstance_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_GameInstance;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialMetricsDisplay_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SpatialMetricsDisplay;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialMetrics_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SpatialMetrics;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EntityPool_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_EntityPool;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SnapshotManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SnapshotManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_StaticComponentView_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_StaticComponentView;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PackageMap_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_PackageMap;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PlayerSpawner_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_PlayerSpawner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GlobalStateManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_GlobalStateManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ClassInfoManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ClassInfoManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorGroupManager_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ActorGroupManager;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Receiver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Receiver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Sender_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Sender;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Dispatcher_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Dispatcher;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Connection_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Connection;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialNetDriver_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UIpNetDriver,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USpatialNetDriver_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USpatialNetDriver_OnLevelAddedToWorld, "OnLevelAddedToWorld" }, // 1813012468
		{ &Z_Construct_UFunction_USpatialNetDriver_OnMapLoaded, "OnMapLoaded" }, // 2233756002
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialNetDriver.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GameInstance_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GameInstance = { "GameInstance", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, GameInstance), Z_Construct_UClass_USpatialGameInstance_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GameInstance_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GameInstance_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetricsDisplay_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetricsDisplay = { "SpatialMetricsDisplay", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, SpatialMetricsDisplay), Z_Construct_UClass_ASpatialMetricsDisplay_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetricsDisplay_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetricsDisplay_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetrics_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetrics = { "SpatialMetrics", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, SpatialMetrics), Z_Construct_UClass_USpatialMetrics_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetrics_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetrics_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_EntityPool_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_EntityPool = { "EntityPool", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, EntityPool), Z_Construct_UClass_UEntityPool_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_EntityPool_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_EntityPool_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SnapshotManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SnapshotManager = { "SnapshotManager", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, SnapshotManager), Z_Construct_UClass_USnapshotManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SnapshotManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SnapshotManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_StaticComponentView_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_StaticComponentView = { "StaticComponentView", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, StaticComponentView), Z_Construct_UClass_USpatialStaticComponentView_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_StaticComponentView_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_StaticComponentView_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PackageMap_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PackageMap = { "PackageMap", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, PackageMap), Z_Construct_UClass_USpatialPackageMapClient_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PackageMap_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PackageMap_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PlayerSpawner_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PlayerSpawner = { "PlayerSpawner", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, PlayerSpawner), Z_Construct_UClass_USpatialPlayerSpawner_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PlayerSpawner_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PlayerSpawner_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GlobalStateManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GlobalStateManager = { "GlobalStateManager", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, GlobalStateManager), Z_Construct_UClass_UGlobalStateManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GlobalStateManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GlobalStateManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ClassInfoManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ClassInfoManager = { "ClassInfoManager", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, ClassInfoManager), Z_Construct_UClass_USpatialClassInfoManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ClassInfoManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ClassInfoManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ActorGroupManager_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ActorGroupManager = { "ActorGroupManager", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, ActorGroupManager), Z_Construct_UClass_UActorGroupManager_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ActorGroupManager_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ActorGroupManager_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Receiver_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Receiver = { "Receiver", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, Receiver), Z_Construct_UClass_USpatialReceiver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Receiver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Receiver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Sender_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Sender = { "Sender", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, Sender), Z_Construct_UClass_USpatialSender_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Sender_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Sender_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Dispatcher_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Dispatcher = { "Dispatcher", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, Dispatcher), Z_Construct_UClass_USpatialDispatcher_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Dispatcher_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Dispatcher_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Connection_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetDriver.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Connection = { "Connection", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetDriver, Connection), Z_Construct_UClass_USpatialWorkerConnection_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Connection_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Connection_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialNetDriver_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GameInstance,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetricsDisplay,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SpatialMetrics,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_EntityPool,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_SnapshotManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_StaticComponentView,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PackageMap,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_PlayerSpawner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_GlobalStateManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ClassInfoManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_ActorGroupManager,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Receiver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Sender,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Dispatcher,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetDriver_Statics::NewProp_Connection,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialNetDriver_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialNetDriver>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialNetDriver_Statics::ClassParams = {
		&USpatialNetDriver::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_USpatialNetDriver_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		ARRAY_COUNT(FuncInfo),
		ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::PropPointers),
		0,
		0x001000ACu,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialNetDriver_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialNetDriver_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialNetDriver()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialNetDriver_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialNetDriver, 1840441397);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialNetDriver>()
	{
		return USpatialNetDriver::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialNetDriver(Z_Construct_UClass_USpatialNetDriver, &USpatialNetDriver::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialNetDriver"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialNetDriver);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
