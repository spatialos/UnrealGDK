// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/SpatialGDKSettings.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialGDKSettings() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialGDKSettings_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialGDKSettings();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FActorGroupInfo();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FWorkerType();
// End Cross Module References
	void USpatialGDKSettings::StaticRegisterNativesUSpatialGDKSettings()
	{
	}
	UClass* Z_Construct_UClass_USpatialGDKSettings_NoRegister()
	{
		return USpatialGDKSettings::StaticClass();
	}
	struct Z_Construct_UClass_USpatialGDKSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ServerWorkerTypes_MetaData[];
#endif
		static const UE4CodeGen_Private::FSetPropertyParams NewProp_ServerWorkerTypes;
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_ServerWorkerTypes_ElementProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorGroups_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_ActorGroups;
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_ActorGroups_Key_KeyProp;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_ActorGroups_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnableOffloading_MetaData[];
#endif
		static void NewProp_bEnableOffloading_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnableOffloading;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DefaultWorkerType_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_DefaultWorkerType;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DevelopmentDeploymentToConnect_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_DevelopmentDeploymentToConnect;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DevelopmentAuthenticationToken_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_DevelopmentAuthenticationToken;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bUseDevelopmentAuthenticationFlow_MetaData[];
#endif
		static void NewProp_bUseDevelopmentAuthenticationFlow_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bUseDevelopmentAuthenticationFlow;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DefaultReceptionistHost_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_DefaultReceptionistHost;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bPackRPCs_MetaData[];
#endif
		static void NewProp_bPackRPCs_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bPackRPCs;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnableServerQBI_MetaData[];
#endif
		static void NewProp_bEnableServerQBI_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnableServerQBI;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_MaxDynamicallyAttachedSubobjectsPerClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_MaxDynamicallyAttachedSubobjectsPerClass;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bBatchSpatialPositionUpdates_MetaData[];
#endif
		static void NewProp_bBatchSpatialPositionUpdates_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bBatchSpatialPositionUpdates;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bCheckRPCOrder_MetaData[];
#endif
		static void NewProp_bCheckRPCOrder_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bCheckRPCOrder;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bUseFrameTimeAsLoad_MetaData[];
#endif
		static void NewProp_bUseFrameTimeAsLoad_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bUseFrameTimeAsLoad;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_MetricsReportRate_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_MetricsReportRate;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnableMetricsDisplay_MetaData[];
#endif
		static void NewProp_bEnableMetricsDisplay_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnableMetricsDisplay;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnableMetrics_MetaData[];
#endif
		static void NewProp_bEnableMetrics_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnableMetrics;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PositionDistanceThreshold_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_PositionDistanceThreshold;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PositionUpdateFrequency_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_PositionUpdateFrequency;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bUsingQBI_MetaData[];
#endif
		static void NewProp_bUsingQBI_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bUsingQBI;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_MaxNetCullDistanceSquared_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_MaxNetCullDistanceSquared;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bEnableHandover_MetaData[];
#endif
		static void NewProp_bEnableHandover_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bEnableHandover;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_OpsUpdateRate_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_OpsUpdateRate;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EntityCreationRateLimit_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_EntityCreationRateLimit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorReplicationRateLimit_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_ActorReplicationRateLimit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_HeartbeatTimeoutSeconds_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_HeartbeatTimeoutSeconds;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_HeartbeatIntervalSeconds_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_HeartbeatIntervalSeconds;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EntityPoolRefreshCount_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_EntityPoolRefreshCount;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EntityPoolRefreshThreshold_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_EntityPoolRefreshThreshold;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EntityPoolInitialReservationCount_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_EntityPoolInitialReservationCount;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialGDKSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SpatialGDKSettings.h" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes_MetaData[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Available server worker types." },
	};
#endif
	const UE4CodeGen_Private::FSetPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes = { "ServerWorkerTypes", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Set, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, ServerWorkerTypes), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes_MetaData)) };
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes_ElementProp = { "ServerWorkerTypes", nullptr, (EPropertyFlags)0x0000000000004000, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_MetaData[] = {
		{ "Category", "Offloading" },
		{ "EditCondition", "bEnableOffloading" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Actor Group configuration." },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups = { "ActorGroups", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, ActorGroups), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_MetaData)) };
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_Key_KeyProp = { "ActorGroups_Key", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_ValueProp = { "ActorGroups", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, Z_Construct_UScriptStruct_FActorGroupInfo, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading_MetaData[] = {
		{ "Category", "Offloading" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Enable running different server worker types to split the simulation by Actor Groups." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bEnableOffloading = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading = { "bEnableOffloading", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultWorkerType_MetaData[] = {
		{ "Category", "Offloading" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Single server worker type to launch when offloading is disabled, fallback server worker type when offloading is enabled (owns all actor classes by default)." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultWorkerType = { "DefaultWorkerType", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, DefaultWorkerType), Z_Construct_UScriptStruct_FWorkerType, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultWorkerType_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultWorkerType_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentDeploymentToConnect_MetaData[] = {
		{ "Category", "Cloud Connection" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "The deployment to connect to when using the Development Authentication Flow. If left empty, it uses the first available one (order not guaranteed when there are multiple items). The deployment needs to be tagged with 'dev_login'." },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentDeploymentToConnect = { "DevelopmentDeploymentToConnect", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, DevelopmentDeploymentToConnect), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentDeploymentToConnect_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentDeploymentToConnect_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentAuthenticationToken_MetaData[] = {
		{ "Category", "Cloud Connection" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "The token created using 'spatial project auth dev-auth-token'" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentAuthenticationToken = { "DevelopmentAuthenticationToken", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, DevelopmentAuthenticationToken), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentAuthenticationToken_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentAuthenticationToken_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow_MetaData[] = {
		{ "Category", "Cloud Connection" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "If the Development Authentication Flow is used, the client will try to connect to the cloud rather than local deployment." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bUseDevelopmentAuthenticationFlow = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow = { "bUseDevelopmentAuthenticationFlow", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultReceptionistHost_MetaData[] = {
		{ "Category", "Local Connection" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "The receptionist host to use if no 'receptionistHost' argument is passed to the command line." },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultReceptionistHost = { "DefaultReceptionistHost", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, DefaultReceptionistHost), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultReceptionistHost_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultReceptionistHost_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs_MetaData[] = {
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Pack RPCs sent during the same frame into a single update." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bPackRPCs = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs = { "bPackRPCs", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI_MetaData[] = {
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "EXPERIMENTAL - This is a stop-gap until we can better define server interest on system entities.\n      Disabling this is not supported in any type of multi-server environment" },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bEnableServerQBI = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI = { "bEnableServerQBI", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxDynamicallyAttachedSubobjectsPerClass_MetaData[] = {
		{ "Category", "Schema Generation" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Maximum Dynamically Attached Subobjects Per Class" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Maximum number of ActorComponents/Subobjects of the same class that can be attached to an Actor." },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxDynamicallyAttachedSubobjectsPerClass = { "MaxDynamicallyAttachedSubobjectsPerClass", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, MaxDynamicallyAttachedSubobjectsPerClass), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxDynamicallyAttachedSubobjectsPerClass_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxDynamicallyAttachedSubobjectsPerClass_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates_MetaData[] = {
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Batch entity position updates to be processed on a single frame." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bBatchSpatialPositionUpdates = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates = { "bBatchSpatialPositionUpdates", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder_MetaData[] = {
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Include an order index with reliable RPCs and warn if they are executed out of order." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bCheckRPCOrder = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder = { "bCheckRPCOrder", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad_MetaData[] = {
		{ "Category", "Metrics" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "By default the SpatialOS Runtime reports server-worker instance\xe2\x80\x99s load in frames per second (FPS).\nSelect this to switch so it reports as seconds per frame.\nThis value is visible as 'Load' in the Inspector, next to each worker." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bUseFrameTimeAsLoad = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad = { "bUseFrameTimeAsLoad", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MetricsReportRate_MetaData[] = {
		{ "Category", "Metrics" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Metrics Report Rate (seconds)" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Frequency that metrics are reported to SpatialOS." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MetricsReportRate = { "MetricsReportRate", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, MetricsReportRate), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MetricsReportRate_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MetricsReportRate_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay_MetaData[] = {
		{ "Category", "Metrics" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Display server metrics on clients." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bEnableMetricsDisplay = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay = { "bEnableMetricsDisplay", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics_MetaData[] = {
		{ "Category", "Metrics" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Metrics about client and server performance can be reported to SpatialOS to monitor a deployments health." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bEnableMetrics = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics = { "bEnableMetrics", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionDistanceThreshold_MetaData[] = {
		{ "Category", "SpatialOS Position Updates" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Threshold an Actor needs to move, in centimeters, before its SpatialOS Position is updated." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionDistanceThreshold = { "PositionDistanceThreshold", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, PositionDistanceThreshold), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionDistanceThreshold_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionDistanceThreshold_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionUpdateFrequency_MetaData[] = {
		{ "Category", "SpatialOS Position Updates" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Frequency for updating an Actor's SpatialOS Position. Updating position should have a low update rate since it is expensive." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionUpdateFrequency = { "PositionUpdateFrequency", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, PositionUpdateFrequency), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionUpdateFrequency_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionUpdateFrequency_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI_MetaData[] = {
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Query Based Interest is required for level streaming and the AlwaysInterested UPROPERTY specifier to be supported when using spatial networking, however comes at a performance cost for larger-scale projects." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bUsingQBI = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI = { "bUsingQBI", nullptr, (EPropertyFlags)0x0010000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxNetCullDistanceSquared_MetaData[] = {
		{ "Category", "Replication" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Maximum NetCullDistanceSquared value used in Spatial networking. Set to 0.0 to disable. This is temporary and will be removed when the runtime issue is resolved." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxNetCullDistanceSquared = { "MaxNetCullDistanceSquared", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, MaxNetCullDistanceSquared), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxNetCullDistanceSquared_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxNetCullDistanceSquared_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover_MetaData[] = {
		{ "Category", "Replication" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Replicate handover properties between servers, required for zoned worker deployments." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover_SetBit(void* Obj)
	{
		((USpatialGDKSettings*)Obj)->bEnableHandover = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover = { "bEnableHandover", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKSettings), &Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_OpsUpdateRate_MetaData[] = {
		{ "Category", "Replication" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "SpatialOS Network Update Rate" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the rate, in number of times per second, at which server-worker instance updates are sent to and received from the SpatialOS Runtime.\nDefault:1000/s" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_OpsUpdateRate = { "OpsUpdateRate", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, OpsUpdateRate), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_OpsUpdateRate_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_OpsUpdateRate_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityCreationRateLimit_MetaData[] = {
		{ "Category", "Replication" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Maximum entities created per tick" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the maximum number of entities created by the SpatialOS Runtime per tick.\n(The SpatialOS Runtime handles entity creation separately from Actor replication to ensure it can handle entity creation requests under load.)\nNote: if you set the value to 0, there is no limit to the number of entities created per tick. However, too many entities created at the same time might overload the SpatialOS Runtime, which can negatively affect your game.\nDefault: `0` per tick  (no limit)" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityCreationRateLimit = { "EntityCreationRateLimit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, EntityCreationRateLimit), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityCreationRateLimit_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityCreationRateLimit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorReplicationRateLimit_MetaData[] = {
		{ "Category", "Replication" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Maximum Actors replicated per tick" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the maximum number of Actors replicated per tick.\nDefault: `0` per tick  (no limit)\n(If you set the value to ` 0`, the SpatialOS Runtime replicates every Actor per tick; this forms a large SpatialOS  world, affecting the performance of both game clients and server-worker instances.)\nYou can use the `stat Spatial` flag when you run project builds to find the number of calls to `ReplicateActor`, and then use this number for reference." },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorReplicationRateLimit = { "ActorReplicationRateLimit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, ActorReplicationRateLimit), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorReplicationRateLimit_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorReplicationRateLimit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatTimeoutSeconds_MetaData[] = {
		{ "Category", "Heartbeat" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Heartbeat Timeout (seconds)" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the maximum amount of time, in seconds, that the server-worker instances wait for a game client to send heartbeat events.\n(If the timeout expires, the game client has disconnected.)" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatTimeoutSeconds = { "HeartbeatTimeoutSeconds", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, HeartbeatTimeoutSeconds), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatTimeoutSeconds_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatTimeoutSeconds_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatIntervalSeconds_MetaData[] = {
		{ "Category", "Heartbeat" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Heartbeat Interval (seconds)" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the amount of time, in seconds, between heartbeat events sent from a game client to notify the server-worker instances that it's connected." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatIntervalSeconds = { "HeartbeatIntervalSeconds", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, HeartbeatIntervalSeconds), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatIntervalSeconds_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatIntervalSeconds_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshCount_MetaData[] = {
		{ "Category", "Entity Pool" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Refresh Count" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies the number of new entity IDs the SpatialOS Runtime reserves when `Pool refresh threshold` triggers a new batch." },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshCount = { "EntityPoolRefreshCount", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, EntityPoolRefreshCount), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshCount_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshCount_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshThreshold_MetaData[] = {
		{ "Category", "Entity Pool" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Pool Refresh Threshold" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "Specifies when the SpatialOS Runtime should reserve a new batch of entity IDs: the value is the number of un-used entity\nIDs left in the entity pool which triggers the SpatialOS Runtime to reserve new entity IDs" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshThreshold = { "EntityPoolRefreshThreshold", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, EntityPoolRefreshThreshold), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshThreshold_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshThreshold_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolInitialReservationCount_MetaData[] = {
		{ "Category", "Entity Pool" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Initial Entity ID Reservation Count" },
		{ "ModuleRelativePath", "Public/SpatialGDKSettings.h" },
		{ "ToolTip", "The number of entity IDs to be reserved when the entity pool is first created. Ensure that the number of entity IDs\nreserved is greater than the number of Actors that you expect the server-worker instances to spawn at game deployment" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolInitialReservationCount = { "EntityPoolInitialReservationCount", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKSettings, EntityPoolInitialReservationCount), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolInitialReservationCount_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolInitialReservationCount_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialGDKSettings_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ServerWorkerTypes_ElementProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorGroups_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableOffloading,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultWorkerType,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentDeploymentToConnect,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DevelopmentAuthenticationToken,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseDevelopmentAuthenticationFlow,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_DefaultReceptionistHost,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bPackRPCs,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableServerQBI,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxDynamicallyAttachedSubobjectsPerClass,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bBatchSpatialPositionUpdates,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bCheckRPCOrder,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUseFrameTimeAsLoad,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MetricsReportRate,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetricsDisplay,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableMetrics,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionDistanceThreshold,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_PositionUpdateFrequency,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bUsingQBI,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_MaxNetCullDistanceSquared,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_bEnableHandover,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_OpsUpdateRate,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityCreationRateLimit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_ActorReplicationRateLimit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatTimeoutSeconds,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_HeartbeatIntervalSeconds,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshCount,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolRefreshThreshold,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKSettings_Statics::NewProp_EntityPoolInitialReservationCount,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialGDKSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialGDKSettings>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialGDKSettings_Statics::ClassParams = {
		&USpatialGDKSettings::StaticClass,
		"SpatialGDKSettings",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialGDKSettings_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::PropPointers),
		0,
		0x001000A6u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialGDKSettings_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKSettings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialGDKSettings()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialGDKSettings_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialGDKSettings, 2551050758);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialGDKSettings>()
	{
		return USpatialGDKSettings::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialGDKSettings(Z_Construct_UClass_USpatialGDKSettings, &USpatialGDKSettings::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialGDKSettings"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialGDKSettings);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
