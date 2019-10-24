// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDKEditor/Public/SpatialGDKEditorSettings.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialGDKEditorSettings() {}
// Cross Module References
	SPATIALGDKEDITOR_API UEnum* Z_Construct_UEnum_SpatialGDKEditor_ERegionCode();
	UPackage* Z_Construct_UPackage__Script_SpatialGDKEditor();
	SPATIALGDKEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription();
	SPATIALGDKEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FWorkerTypeLaunchSection();
	SPATIALGDKEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FWorldLaunchSection();
	SPATIALGDKEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FLoginRateLimitSection();
	SPATIALGDKEDITOR_API UScriptStruct* Z_Construct_UScriptStruct_FWorkerPermissionsSection();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FIntPoint();
	SPATIALGDKEDITOR_API UClass* Z_Construct_UClass_USpatialGDKEditorSettings_NoRegister();
	SPATIALGDKEDITOR_API UClass* Z_Construct_UClass_USpatialGDKEditorSettings();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	ENGINE_API UScriptStruct* Z_Construct_UScriptStruct_FFilePath();
// End Cross Module References
	static UEnum* ERegionCode_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_SpatialGDKEditor_ERegionCode, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("ERegionCode"));
		}
		return Singleton;
	}
	template<> SPATIALGDKEDITOR_API UEnum* StaticEnum<ERegionCode::Type>()
	{
		return ERegionCode_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_ERegionCode(ERegionCode_StaticEnum, TEXT("/Script/SpatialGDKEditor"), TEXT("ERegionCode"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_SpatialGDKEditor_ERegionCode_Hash() { return 2627874497U; }
	UEnum* Z_Construct_UEnum_SpatialGDKEditor_ERegionCode()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("ERegionCode"), 0, Get_Z_Construct_UEnum_SpatialGDKEditor_ERegionCode_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "ERegionCode::US", (int64)ERegionCode::US },
				{ "ERegionCode::EU", (int64)ERegionCode::EU },
				{ "ERegionCode::AP", (int64)ERegionCode::AP },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
				{ "ToolTip", "Enumerates available Region Codes" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
				nullptr,
				"ERegionCode",
				"ERegionCode::Type",
				Enumerators,
				ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::Namespaced,
				METADATA_PARAMS(Enum_MetaDataParams, ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
class UScriptStruct* FSpatialLaunchConfigDescription::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDKEDITOR_API uint32 Get_Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("SpatialLaunchConfigDescription"), sizeof(FSpatialLaunchConfigDescription), Get_Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Hash());
	}
	return Singleton;
}
template<> SPATIALGDKEDITOR_API UScriptStruct* StaticStruct<FSpatialLaunchConfigDescription>()
{
	return FSpatialLaunchConfigDescription::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FSpatialLaunchConfigDescription(FSpatialLaunchConfigDescription::StaticStruct, TEXT("/Script/SpatialGDKEditor"), TEXT("SpatialLaunchConfigDescription"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFSpatialLaunchConfigDescription
{
	FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFSpatialLaunchConfigDescription()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("SpatialLaunchConfigDescription")),new UScriptStruct::TCppStructOps<FSpatialLaunchConfigDescription>);
	}
} ScriptStruct_SpatialGDKEditor_StaticRegisterNativesFSpatialLaunchConfigDescription;
	struct Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ServerWorkers_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_ServerWorkers;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_ServerWorkers_Inner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_World_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_World;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Template_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Template;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FSpatialLaunchConfigDescription>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Worker-specific configuration parameters." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers = { "ServerWorkers", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FSpatialLaunchConfigDescription, ServerWorkers), METADATA_PARAMS(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers_MetaData)) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers_Inner = { "ServerWorkers", nullptr, (EPropertyFlags)0x0000000000004000, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FWorkerTypeLaunchSection, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_World_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Configuration for the simulated world." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_World = { "World", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FSpatialLaunchConfigDescription, World), Z_Construct_UScriptStruct_FWorldLaunchSection, METADATA_PARAMS(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_World_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_World_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_Template_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Deployment template." },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_Template = { "Template", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FSpatialLaunchConfigDescription, Template), METADATA_PARAMS(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_Template_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_Template_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_ServerWorkers_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_World,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::NewProp_Template,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
		nullptr,
		&NewStructOps,
		"SpatialLaunchConfigDescription",
		sizeof(FSpatialLaunchConfigDescription),
		alignof(FSpatialLaunchConfigDescription),
		Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("SpatialLaunchConfigDescription"), sizeof(FSpatialLaunchConfigDescription), Get_Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription_Hash() { return 1732561957U; }
class UScriptStruct* FWorkerTypeLaunchSection::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDKEDITOR_API uint32 Get_Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("WorkerTypeLaunchSection"), sizeof(FWorkerTypeLaunchSection), Get_Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Hash());
	}
	return Singleton;
}
template<> SPATIALGDKEDITOR_API UScriptStruct* StaticStruct<FWorkerTypeLaunchSection>()
{
	return FWorkerTypeLaunchSection::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FWorkerTypeLaunchSection(FWorkerTypeLaunchSection::StaticStruct, TEXT("/Script/SpatialGDKEditor"), TEXT("WorkerTypeLaunchSection"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerTypeLaunchSection
{
	FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerTypeLaunchSection()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("WorkerTypeLaunchSection")),new UScriptStruct::TCppStructOps<FWorkerTypeLaunchSection>);
	}
} ScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerTypeLaunchSection;
	struct Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bManualWorkerConnectionOnly_MetaData[];
#endif
		static void NewProp_bManualWorkerConnectionOnly_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bManualWorkerConnectionOnly;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Flags_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_Flags;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Flags_Key_KeyProp;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Flags_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NumEditorInstances_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_NumEditorInstances;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Rows_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Rows;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Columns_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Columns;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LoginRateLimit_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_LoginRateLimit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bLoginRateLimitEnabled_MetaData[];
#endif
		static void NewProp_bLoginRateLimitEnabled_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bLoginRateLimitEnabled;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_MaxConnectionCapacityLimit_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_MaxConnectionCapacityLimit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WorkerPermissions_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_WorkerPermissions;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WorkerTypeName_MetaData[];
#endif
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_WorkerTypeName;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FWorkerTypeLaunchSection>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Manual worker connection only" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Determines if the worker instance is launched manually or by SpatialOS." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly_SetBit(void* Obj)
	{
		((FWorkerTypeLaunchSection*)Obj)->bManualWorkerConnectionOnly = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly = { "bManualWorkerConnectionOnly", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerTypeLaunchSection), &Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Flags" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Flags defined for a worker instance." },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags = { "Flags", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, Flags), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_Key_KeyProp = { "Flags_Key", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_ValueProp = { "Flags", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_NumEditorInstances_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ClampMin", "0" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Instances to launch in editor" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Number of instances to launch when playing in editor." },
		{ "UIMin", "0" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_NumEditorInstances = { "NumEditorInstances", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, NumEditorInstances), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_NumEditorInstances_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_NumEditorInstances_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Rows_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ClampMin", "1" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Rectangle grid row count" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Number of rows in the rectangle grid load balancing config." },
		{ "UIMin", "1" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Rows = { "Rows", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, Rows), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Rows_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Rows_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Columns_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ClampMin", "1" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Rectangle grid column count" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Number of columns in the rectangle grid load balancing config." },
		{ "UIMin", "1" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Columns = { "Columns", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, Columns), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Columns_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Columns_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_LoginRateLimit_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "EditCondition", "bLoginRateLimitEnabled" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Login rate limiting configuration." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_LoginRateLimit = { "LoginRateLimit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, LoginRateLimit), Z_Construct_UScriptStruct_FLoginRateLimitSection, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_LoginRateLimit_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_LoginRateLimit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Login rate limit enabled" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Enable connection rate limiting." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled_SetBit(void* Obj)
	{
		((FWorkerTypeLaunchSection*)Obj)->bLoginRateLimitEnabled = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled = { "bLoginRateLimitEnabled", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerTypeLaunchSection), &Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_MaxConnectionCapacityLimit_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ClampMin", "0" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Max connection capacity limit (0 = unlimited capacity)" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Defines the maximum number of worker instances that can connect." },
		{ "UIMin", "0" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_MaxConnectionCapacityLimit = { "MaxConnectionCapacityLimit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, MaxConnectionCapacityLimit), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_MaxConnectionCapacityLimit_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_MaxConnectionCapacityLimit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerPermissions_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Defines the worker instance's permissions." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerPermissions = { "WorkerPermissions", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, WorkerPermissions), Z_Construct_UScriptStruct_FWorkerPermissionsSection, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerPermissions_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerPermissions_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerTypeName_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The name of the worker type, defined in the filename of its spatialos.<worker_type>.worker.json file." },
	};
#endif
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerTypeName = { "WorkerTypeName", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerTypeLaunchSection, WorkerTypeName), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerTypeName_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerTypeName_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bManualWorkerConnectionOnly,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Flags_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_NumEditorInstances,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Rows,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_Columns,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_LoginRateLimit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_bLoginRateLimitEnabled,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_MaxConnectionCapacityLimit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerPermissions,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::NewProp_WorkerTypeName,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
		nullptr,
		&NewStructOps,
		"WorkerTypeLaunchSection",
		sizeof(FWorkerTypeLaunchSection),
		alignof(FWorkerTypeLaunchSection),
		Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FWorkerTypeLaunchSection()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("WorkerTypeLaunchSection"), sizeof(FWorkerTypeLaunchSection), Get_Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FWorkerTypeLaunchSection_Hash() { return 3742276115U; }
class UScriptStruct* FLoginRateLimitSection::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDKEDITOR_API uint32 Get_Z_Construct_UScriptStruct_FLoginRateLimitSection_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FLoginRateLimitSection, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("LoginRateLimitSection"), sizeof(FLoginRateLimitSection), Get_Z_Construct_UScriptStruct_FLoginRateLimitSection_Hash());
	}
	return Singleton;
}
template<> SPATIALGDKEDITOR_API UScriptStruct* StaticStruct<FLoginRateLimitSection>()
{
	return FLoginRateLimitSection::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FLoginRateLimitSection(FLoginRateLimitSection::StaticStruct, TEXT("/Script/SpatialGDKEditor"), TEXT("LoginRateLimitSection"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFLoginRateLimitSection
{
	FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFLoginRateLimitSection()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("LoginRateLimitSection")),new UScriptStruct::TCppStructOps<FLoginRateLimitSection>);
	}
} ScriptStruct_SpatialGDKEditor_StaticRegisterNativesFLoginRateLimitSection;
	struct Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RequestsPerDuration_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_RequestsPerDuration;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Duration_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Duration;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FLoginRateLimitSection>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_RequestsPerDuration_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ClampMin", "1" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The connection request limit for the duration." },
		{ "UIMin", "1" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_RequestsPerDuration = { "RequestsPerDuration", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FLoginRateLimitSection, RequestsPerDuration), METADATA_PARAMS(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_RequestsPerDuration_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_RequestsPerDuration_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_Duration_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The duration for which worker connection requests will be limited." },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_Duration = { "Duration", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FLoginRateLimitSection, Duration), METADATA_PARAMS(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_Duration_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_Duration_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_RequestsPerDuration,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::NewProp_Duration,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
		nullptr,
		&NewStructOps,
		"LoginRateLimitSection",
		sizeof(FLoginRateLimitSection),
		alignof(FLoginRateLimitSection),
		Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FLoginRateLimitSection()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FLoginRateLimitSection_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("LoginRateLimitSection"), sizeof(FLoginRateLimitSection), Get_Z_Construct_UScriptStruct_FLoginRateLimitSection_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FLoginRateLimitSection_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FLoginRateLimitSection_Hash() { return 1254298807U; }
class UScriptStruct* FWorkerPermissionsSection::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDKEDITOR_API uint32 Get_Z_Construct_UScriptStruct_FWorkerPermissionsSection_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FWorkerPermissionsSection, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("WorkerPermissionsSection"), sizeof(FWorkerPermissionsSection), Get_Z_Construct_UScriptStruct_FWorkerPermissionsSection_Hash());
	}
	return Singleton;
}
template<> SPATIALGDKEDITOR_API UScriptStruct* StaticStruct<FWorkerPermissionsSection>()
{
	return FWorkerPermissionsSection::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FWorkerPermissionsSection(FWorkerPermissionsSection::StaticStruct, TEXT("/Script/SpatialGDKEditor"), TEXT("WorkerPermissionsSection"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerPermissionsSection
{
	FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerPermissionsSection()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("WorkerPermissionsSection")),new UScriptStruct::TCppStructOps<FWorkerPermissionsSection>);
	}
} ScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorkerPermissionsSection;
	struct Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Components_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Components;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Components_Inner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bAllowEntityQuery_MetaData[];
#endif
		static void NewProp_bAllowEntityQuery_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bAllowEntityQuery;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bAllowEntityDeletion_MetaData[];
#endif
		static void NewProp_bAllowEntityDeletion_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bAllowEntityDeletion;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bAllowEntityCreation_MetaData[];
#endif
		static void NewProp_bAllowEntityCreation_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bAllowEntityCreation;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bAllPermissions_MetaData[];
#endif
		static void NewProp_bAllPermissions_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bAllPermissions;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FWorkerPermissionsSection>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Component queries" },
		{ "EditCondition", "!bAllPermissions" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Specifies which components can be returned in the query result." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components = { "Components", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorkerPermissionsSection, Components), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components_Inner = { "Components", nullptr, (EPropertyFlags)0x0000000000004000, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Allow entity query" },
		{ "EditCondition", "!bAllPermissions" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Controls which components can be returned from entity queries that the worker instance performs. If an entity query specifies other components to be returned, the query will fail." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery_SetBit(void* Obj)
	{
		((FWorkerPermissionsSection*)Obj)->bAllowEntityQuery = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery = { "bAllowEntityQuery", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerPermissionsSection), &Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Allow entity deletion" },
		{ "EditCondition", "!bAllPermissions" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Enables a worker instance to delete entities." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion_SetBit(void* Obj)
	{
		((FWorkerPermissionsSection*)Obj)->bAllowEntityDeletion = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion = { "bAllowEntityDeletion", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerPermissionsSection), &Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Allow entity creation" },
		{ "EditCondition", "!bAllPermissions" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Enables a worker instance to create new entities." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation_SetBit(void* Obj)
	{
		((FWorkerPermissionsSection*)Obj)->bAllowEntityCreation = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation = { "bAllowEntityCreation", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerPermissionsSection), &Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "All" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Gives all permissions to a worker instance." },
	};
#endif
	void Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions_SetBit(void* Obj)
	{
		((FWorkerPermissionsSection*)Obj)->bAllPermissions = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions = { "bAllPermissions", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(FWorkerPermissionsSection), &Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions_SetBit, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_Components_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityQuery,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityDeletion,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllowEntityCreation,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::NewProp_bAllPermissions,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
		nullptr,
		&NewStructOps,
		"WorkerPermissionsSection",
		sizeof(FWorkerPermissionsSection),
		alignof(FWorkerPermissionsSection),
		Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FWorkerPermissionsSection()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FWorkerPermissionsSection_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("WorkerPermissionsSection"), sizeof(FWorkerPermissionsSection), Get_Z_Construct_UScriptStruct_FWorkerPermissionsSection_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FWorkerPermissionsSection_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FWorkerPermissionsSection_Hash() { return 4014290101U; }
class UScriptStruct* FWorldLaunchSection::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDKEDITOR_API uint32 Get_Z_Construct_UScriptStruct_FWorldLaunchSection_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FWorldLaunchSection, Z_Construct_UPackage__Script_SpatialGDKEditor(), TEXT("WorldLaunchSection"), sizeof(FWorldLaunchSection), Get_Z_Construct_UScriptStruct_FWorldLaunchSection_Hash());
	}
	return Singleton;
}
template<> SPATIALGDKEDITOR_API UScriptStruct* StaticStruct<FWorldLaunchSection>()
{
	return FWorldLaunchSection::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FWorldLaunchSection(FWorldLaunchSection::StaticStruct, TEXT("/Script/SpatialGDKEditor"), TEXT("WorldLaunchSection"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorldLaunchSection
{
	FScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorldLaunchSection()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("WorldLaunchSection")),new UScriptStruct::TCppStructOps<FWorldLaunchSection>);
	}
} ScriptStruct_SpatialGDKEditor_StaticRegisterNativesFWorldLaunchSection;
	struct Z_Construct_UScriptStruct_FWorldLaunchSection_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LegacyJavaParams_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_LegacyJavaParams;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_LegacyJavaParams_Key_KeyProp;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_LegacyJavaParams_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LegacyFlags_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_LegacyFlags;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_LegacyFlags_Key_KeyProp;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_LegacyFlags_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SnapshotWritePeriodSeconds_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_SnapshotWritePeriodSeconds;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_StreamingQueryIntervalSeconds_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_StreamingQueryIntervalSeconds;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ChunkEdgeLengthMeters_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ChunkEdgeLengthMeters;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Dimensions_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_Dimensions;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FWorldLaunchSection>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Legacy Java parameters" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Legacy JVM configurations." },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams = { "LegacyJavaParams", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, LegacyJavaParams), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_Key_KeyProp = { "LegacyJavaParams_Key", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_ValueProp = { "LegacyJavaParams", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Legacy non-worker flag configurations." },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags = { "LegacyFlags", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, LegacyFlags), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_Key_KeyProp = { "LegacyFlags_Key", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_ValueProp = { "LegacyFlags", nullptr, (EPropertyFlags)0x0000000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_SnapshotWritePeriodSeconds_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Snapshot write period in seconds" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The frequency in seconds to write snapshots of the simulated world." },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_SnapshotWritePeriodSeconds = { "SnapshotWritePeriodSeconds", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, SnapshotWritePeriodSeconds), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_SnapshotWritePeriodSeconds_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_SnapshotWritePeriodSeconds_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_StreamingQueryIntervalSeconds_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Streaming query interval in seconds" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The time in seconds between streaming query updates." },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_StreamingQueryIntervalSeconds = { "StreamingQueryIntervalSeconds", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, StreamingQueryIntervalSeconds), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_StreamingQueryIntervalSeconds_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_StreamingQueryIntervalSeconds_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_ChunkEdgeLengthMeters_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Chunk edge length in meters" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The size of the grid squares that the world is divided into, in \xe2\x80\x9cworld units\xe2\x80\x9d (an arbitrary unit that worker instances can interpret as they choose)." },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_ChunkEdgeLengthMeters = { "ChunkEdgeLengthMeters", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, ChunkEdgeLengthMeters), METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_ChunkEdgeLengthMeters_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_ChunkEdgeLengthMeters_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_Dimensions_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Simulation dimensions in meters" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "The size of the simulation, in meters, for the auto-generated launch configuration file." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_Dimensions = { "Dimensions", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FWorldLaunchSection, Dimensions), Z_Construct_UScriptStruct_FIntPoint, METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_Dimensions_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_Dimensions_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyJavaParams_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_LegacyFlags_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_SnapshotWritePeriodSeconds,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_StreamingQueryIntervalSeconds,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_ChunkEdgeLengthMeters,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::NewProp_Dimensions,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
		nullptr,
		&NewStructOps,
		"WorldLaunchSection",
		sizeof(FWorldLaunchSection),
		alignof(FWorldLaunchSection),
		Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FWorldLaunchSection()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FWorldLaunchSection_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDKEditor();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("WorldLaunchSection"), sizeof(FWorldLaunchSection), Get_Z_Construct_UScriptStruct_FWorldLaunchSection_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FWorldLaunchSection_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FWorldLaunchSection_Hash() { return 377975388U; }
	void USpatialGDKEditorSettings::StaticRegisterNativesUSpatialGDKEditorSettings()
	{
	}
	UClass* Z_Construct_UClass_USpatialGDKEditorSettings_NoRegister()
	{
		return USpatialGDKEditorSettings::StaticClass();
	}
	struct Z_Construct_UClass_USpatialGDKEditorSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LaunchConfigDesc_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_LaunchConfigDesc;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NumberOfSimulatedPlayers_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_NumberOfSimulatedPlayers;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SimulatedPlayerDeploymentName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_SimulatedPlayerDeploymentName;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bSimulatedPlayersIsEnabled_MetaData[];
#endif
		static void NewProp_bSimulatedPlayersIsEnabled_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSimulatedPlayersIsEnabled;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SimulatedPlayerDeploymentRegionCode_MetaData[];
#endif
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_SimulatedPlayerDeploymentRegionCode;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PrimaryDeploymentRegionCode_MetaData[];
#endif
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_PrimaryDeploymentRegionCode;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SnapshotPath_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_SnapshotPath;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PrimaryLaunchConfigPath_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_PrimaryLaunchConfigPath;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_PrimaryDeploymentName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_PrimaryDeploymentName;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_AssemblyName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_AssemblyName;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ProjectName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ProjectName;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialOSCommandLineLaunchFlags_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_SpatialOSCommandLineLaunchFlags;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_SpatialOSCommandLineLaunchFlags_Inner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialOSSnapshotFile_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_SpatialOSSnapshotFile;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bAutoStartLocalDeployment_MetaData[];
#endif
		static void NewProp_bAutoStartLocalDeployment_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bAutoStartLocalDeployment;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bStopSpatialOnExit_MetaData[];
#endif
		static void NewProp_bStopSpatialOnExit_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bStopSpatialOnExit;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialOSLaunchConfig_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_SpatialOSLaunchConfig;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bGenerateDefaultLaunchConfig_MetaData[];
#endif
		static void NewProp_bGenerateDefaultLaunchConfig_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bGenerateDefaultLaunchConfig;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bDeleteDynamicEntities_MetaData[];
#endif
		static void NewProp_bDeleteDynamicEntities_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bDeleteDynamicEntities;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bShowSpatialServiceButton_MetaData[];
#endif
		static void NewProp_bShowSpatialServiceButton_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bShowSpatialServiceButton;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialGDKEditorSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditor,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SpatialGDKEditorSettings.h" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_LaunchConfigDesc_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Launch configuration file options" },
		{ "EditCondition", "bGenerateDefaultLaunchConfig" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "If you have selected **Auto-generate launch configuration file**, you can change the default options in the file from the drop-down menu." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_LaunchConfigDesc = { "LaunchConfigDesc", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, LaunchConfigDesc), Z_Construct_UScriptStruct_FSpatialLaunchConfigDescription, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_LaunchConfigDesc_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_LaunchConfigDesc_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_NumberOfSimulatedPlayers_MetaData[] = {
		{ "Category", "Simulated Players" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Number of simulated players" },
		{ "EditCondition", "bSimulatedPlayersIsEnabled" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_NumberOfSimulatedPlayers = { "NumberOfSimulatedPlayers", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, NumberOfSimulatedPlayers), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_NumberOfSimulatedPlayers_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_NumberOfSimulatedPlayers_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentName_MetaData[] = {
		{ "Category", "Simulated Players" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Deployment name" },
		{ "EditCondition", "bSimulatedPlayersIsEnabled" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentName = { "SimulatedPlayerDeploymentName", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SimulatedPlayerDeploymentName), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentName_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentName_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled_MetaData[] = {
		{ "Category", "Simulated Players" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Include simulated players" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bSimulatedPlayersIsEnabled = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled = { "bSimulatedPlayersIsEnabled", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentRegionCode_MetaData[] = {
		{ "Category", "Simulated Players" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Region" },
		{ "EditCondition", "bSimulatedPlayersIsEnabled" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentRegionCode = { "SimulatedPlayerDeploymentRegionCode", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SimulatedPlayerDeploymentRegionCode), Z_Construct_UEnum_SpatialGDKEditor_ERegionCode, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentRegionCode_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentRegionCode_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentRegionCode_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Region" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentRegionCode = { "PrimaryDeploymentRegionCode", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, PrimaryDeploymentRegionCode), Z_Construct_UEnum_SpatialGDKEditor_ERegionCode, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentRegionCode_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentRegionCode_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SnapshotPath_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Snapshot path" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SnapshotPath = { "SnapshotPath", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SnapshotPath), Z_Construct_UScriptStruct_FFilePath, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SnapshotPath_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SnapshotPath_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryLaunchConfigPath_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Cloud launch configuration path" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryLaunchConfigPath = { "PrimaryLaunchConfigPath", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, PrimaryLaunchConfigPath), Z_Construct_UScriptStruct_FFilePath, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryLaunchConfigPath_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryLaunchConfigPath_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentName_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Deployment name" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentName = { "PrimaryDeploymentName", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, PrimaryDeploymentName), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentName_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentName_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_AssemblyName_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Assembly name" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_AssemblyName = { "AssemblyName", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, AssemblyName), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_AssemblyName_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_AssemblyName_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_ProjectName_MetaData[] = {
		{ "Category", "Cloud" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "SpatialOS project" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_ProjectName = { "ProjectName", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, ProjectName), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_ProjectName_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_ProjectName_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Command line flags for local launch" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Add flags to the `spatial local launch` command; they alter the deployment\xe2\x80\x99s behavior. Select the trash icon to remove all the flags." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags = { "SpatialOSCommandLineLaunchFlags", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SpatialOSCommandLineLaunchFlags), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags_Inner = { "SpatialOSCommandLineLaunchFlags", nullptr, (EPropertyFlags)0x0000000000004000, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSSnapshotFile_MetaData[] = {
		{ "Category", "Snapshots" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Snapshot file name" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Name of your SpatialOS snapshot file." },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSSnapshotFile = { "SpatialOSSnapshotFile", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SpatialOSSnapshotFile), METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSSnapshotFile_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSSnapshotFile_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Auto-start local deployment" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Start a local SpatialOS deployment when clicking 'Play'." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bAutoStartLocalDeployment = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment = { "bAutoStartLocalDeployment", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Stop local deployment on exit" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Select the check box to stop your game\xe2\x80\x99s local deployment when you shut down Unreal Editor." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bStopSpatialOnExit = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit = { "bStopSpatialOnExit", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSLaunchConfig_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Launch configuration file path" },
		{ "EditCondition", "!bGenerateDefaultLaunchConfig" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "If you are not using auto-generate launch configuration file, specify a launch configuration `.json` file and location here." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSLaunchConfig = { "SpatialOSLaunchConfig", nullptr, (EPropertyFlags)0x0040000000004001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGDKEditorSettings, SpatialOSLaunchConfig), Z_Construct_UScriptStruct_FFilePath, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSLaunchConfig_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSLaunchConfig_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig_MetaData[] = {
		{ "Category", "Launch" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Auto-generate launch configuration file" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Select the check box for the GDK to auto-generate a launch configuration file for your game when you launch a deployment session. If NOT selected, you must specify a launch configuration `.json` file." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bGenerateDefaultLaunchConfig = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig = { "bGenerateDefaultLaunchConfig", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities_MetaData[] = {
		{ "Category", "Play in editor settings" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Delete dynamically spawned entities" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "Select to delete all a server-worker instance\xe2\x80\x99s dynamically-spawned entities when the server-worker instance shuts down. If NOT selected, a new server-worker instance has all of these entities from the former server-worker instance\xe2\x80\x99s session." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bDeleteDynamicEntities = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities = { "bDeleteDynamicEntities", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton_MetaData[] = {
		{ "Category", "General" },
		{ "ConfigRestartRequired", "FALSE" },
		{ "DisplayName", "Show Spatial service button" },
		{ "ModuleRelativePath", "Public/SpatialGDKEditorSettings.h" },
		{ "ToolTip", "If checked, show the Spatial service button on the GDK toolbar which can be used to turn the Spatial service on and off." },
	};
#endif
	void Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton_SetBit(void* Obj)
	{
		((USpatialGDKEditorSettings*)Obj)->bShowSpatialServiceButton = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton = { "bShowSpatialServiceButton", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGDKEditorSettings), &Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialGDKEditorSettings_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_LaunchConfigDesc,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_NumberOfSimulatedPlayers,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentName,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bSimulatedPlayersIsEnabled,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SimulatedPlayerDeploymentRegionCode,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentRegionCode,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SnapshotPath,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryLaunchConfigPath,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_PrimaryDeploymentName,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_AssemblyName,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_ProjectName,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSCommandLineLaunchFlags_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSSnapshotFile,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bAutoStartLocalDeployment,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bStopSpatialOnExit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_SpatialOSLaunchConfig,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bGenerateDefaultLaunchConfig,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bDeleteDynamicEntities,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGDKEditorSettings_Statics::NewProp_bShowSpatialServiceButton,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialGDKEditorSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialGDKEditorSettings>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialGDKEditorSettings_Statics::ClassParams = {
		&USpatialGDKEditorSettings::StaticClass,
		"SpatialGDKEditorSettings",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialGDKEditorSettings_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::PropPointers),
		0,
		0x001000A6u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialGDKEditorSettings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialGDKEditorSettings()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialGDKEditorSettings_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialGDKEditorSettings, 1127718768);
	template<> SPATIALGDKEDITOR_API UClass* StaticClass<USpatialGDKEditorSettings>()
	{
		return USpatialGDKEditorSettings::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialGDKEditorSettings(Z_Construct_UClass_USpatialGDKEditorSettings, &USpatialGDKEditorSettings::StaticClass, TEXT("/Script/SpatialGDKEditor"), TEXT("USpatialGDKEditorSettings"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialGDKEditorSettings);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
