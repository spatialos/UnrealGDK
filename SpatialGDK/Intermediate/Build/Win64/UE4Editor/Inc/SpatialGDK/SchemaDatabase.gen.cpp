// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Utils/SchemaDatabase.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSchemaDatabase() {}
// Cross Module References
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FSubobjectSchemaData();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FActorSchemaData();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData();
	SPATIALGDK_API UClass* Z_Construct_UClass_USchemaDatabase_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USchemaDatabase();
	ENGINE_API UClass* Z_Construct_UClass_UDataAsset();
// End Cross Module References
class UScriptStruct* FSubobjectSchemaData::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FSubobjectSchemaData_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FSubobjectSchemaData, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("SubobjectSchemaData"), sizeof(FSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FSubobjectSchemaData_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FSubobjectSchemaData>()
{
	return FSubobjectSchemaData::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FSubobjectSchemaData(FSubobjectSchemaData::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("SubobjectSchemaData"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFSubobjectSchemaData
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFSubobjectSchemaData()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("SubobjectSchemaData")),new UScriptStruct::TCppStructOps<FSubobjectSchemaData>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFSubobjectSchemaData;
	struct Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DynamicSubobjectComponents_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_DynamicSubobjectComponents;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_DynamicSubobjectComponents_Inner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GeneratedSchemaName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_GeneratedSchemaName;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
		{ "ToolTip", "Schema data related to a Subobject class" },
	};
#endif
	void* Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FSubobjectSchemaData>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents = { "DynamicSubobjectComponents", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FSubobjectSchemaData, DynamicSubobjectComponents), METADATA_PARAMS(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents_MetaData)) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents_Inner = { "DynamicSubobjectComponents", nullptr, (EPropertyFlags)0x0000000000020000, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_GeneratedSchemaName = { "GeneratedSchemaName", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FSubobjectSchemaData, GeneratedSchemaName), METADATA_PARAMS(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_DynamicSubobjectComponents_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::NewProp_GeneratedSchemaName,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"SubobjectSchemaData",
		sizeof(FSubobjectSchemaData),
		alignof(FSubobjectSchemaData),
		Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FSubobjectSchemaData()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FSubobjectSchemaData_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("SubobjectSchemaData"), sizeof(FSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FSubobjectSchemaData_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FSubobjectSchemaData_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FSubobjectSchemaData_Hash() { return 1903690568U; }
class UScriptStruct* FDynamicSubobjectSchemaData::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("DynamicSubobjectSchemaData"), sizeof(FDynamicSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FDynamicSubobjectSchemaData>()
{
	return FDynamicSubobjectSchemaData::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FDynamicSubobjectSchemaData(FDynamicSubobjectSchemaData::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("DynamicSubobjectSchemaData"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFDynamicSubobjectSchemaData
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFDynamicSubobjectSchemaData()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("DynamicSubobjectSchemaData")),new UScriptStruct::TCppStructOps<FDynamicSubobjectSchemaData>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFDynamicSubobjectSchemaData;
	struct Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SchemaComponents_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_SchemaComponents;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FDynamicSubobjectSchemaData>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewProp_SchemaComponents = { "SchemaComponents", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, CPP_ARRAY_DIM(SchemaComponents, FDynamicSubobjectSchemaData), STRUCT_OFFSET(FDynamicSubobjectSchemaData, SchemaComponents), METADATA_PARAMS(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::NewProp_SchemaComponents,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"DynamicSubobjectSchemaData",
		sizeof(FDynamicSubobjectSchemaData),
		alignof(FDynamicSubobjectSchemaData),
		Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("DynamicSubobjectSchemaData"), sizeof(FDynamicSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FDynamicSubobjectSchemaData_Hash() { return 883054219U; }
class UScriptStruct* FActorSchemaData::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FActorSchemaData_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FActorSchemaData, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("ActorSchemaData"), sizeof(FActorSchemaData), Get_Z_Construct_UScriptStruct_FActorSchemaData_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FActorSchemaData>()
{
	return FActorSchemaData::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FActorSchemaData(FActorSchemaData::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("ActorSchemaData"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFActorSchemaData
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFActorSchemaData()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("ActorSchemaData")),new UScriptStruct::TCppStructOps<FActorSchemaData>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFActorSchemaData;
	struct Z_Construct_UScriptStruct_FActorSchemaData_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SubobjectData_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_SubobjectData;
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_SubobjectData_Key_KeyProp;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_SubobjectData_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SchemaComponents_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_SchemaComponents;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GeneratedSchemaName_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_GeneratedSchemaName;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSchemaData_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
		{ "ToolTip", "Schema data related to an Actor class" },
	};
#endif
	void* Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FActorSchemaData>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData = { "SubobjectData", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorSchemaData, SubobjectData), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_MetaData)) };
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_Key_KeyProp = { "SubobjectData_Key", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_ValueProp = { "SubobjectData", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SchemaComponents_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SchemaComponents = { "SchemaComponents", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, CPP_ARRAY_DIM(SchemaComponents, FActorSchemaData), STRUCT_OFFSET(FActorSchemaData, SchemaComponents), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SchemaComponents_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SchemaComponents_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_GeneratedSchemaName = { "GeneratedSchemaName", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorSchemaData, GeneratedSchemaName), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_GeneratedSchemaName_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FActorSchemaData_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SubobjectData_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_SchemaComponents,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSchemaData_Statics::NewProp_GeneratedSchemaName,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FActorSchemaData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"ActorSchemaData",
		sizeof(FActorSchemaData),
		alignof(FActorSchemaData),
		Z_Construct_UScriptStruct_FActorSchemaData_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSchemaData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSchemaData_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSchemaData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FActorSchemaData()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FActorSchemaData_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("ActorSchemaData"), sizeof(FActorSchemaData), Get_Z_Construct_UScriptStruct_FActorSchemaData_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FActorSchemaData_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FActorSchemaData_Hash() { return 701580066U; }
class UScriptStruct* FActorSpecificSubobjectSchemaData::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("ActorSpecificSubobjectSchemaData"), sizeof(FActorSpecificSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FActorSpecificSubobjectSchemaData>()
{
	return FActorSpecificSubobjectSchemaData::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FActorSpecificSubobjectSchemaData(FActorSpecificSubobjectSchemaData::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("ActorSpecificSubobjectSchemaData"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFActorSpecificSubobjectSchemaData
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFActorSpecificSubobjectSchemaData()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("ActorSpecificSubobjectSchemaData")),new UScriptStruct::TCppStructOps<FActorSpecificSubobjectSchemaData>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFActorSpecificSubobjectSchemaData;
	struct Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SchemaComponents_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_SchemaComponents;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Name_MetaData[];
#endif
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_Name;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ClassPath_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ClassPath;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
		{ "ToolTip", "Schema data related to a default Subobject owned by a specific Actor class." },
	};
#endif
	void* Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FActorSpecificSubobjectSchemaData>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_SchemaComponents = { "SchemaComponents", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, CPP_ARRAY_DIM(SchemaComponents, FActorSpecificSubobjectSchemaData), STRUCT_OFFSET(FActorSpecificSubobjectSchemaData, SchemaComponents), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_SchemaComponents_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_Name_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_Name = { "Name", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorSpecificSubobjectSchemaData, Name), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_Name_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_Name_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_ClassPath_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_ClassPath = { "ClassPath", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FActorSpecificSubobjectSchemaData, ClassPath), METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_ClassPath_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_ClassPath_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_SchemaComponents,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_Name,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::NewProp_ClassPath,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"ActorSpecificSubobjectSchemaData",
		sizeof(FActorSpecificSubobjectSchemaData),
		alignof(FActorSpecificSubobjectSchemaData),
		Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("ActorSpecificSubobjectSchemaData"), sizeof(FActorSpecificSubobjectSchemaData), Get_Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FActorSpecificSubobjectSchemaData_Hash() { return 4247962371U; }
	void USchemaDatabase::StaticRegisterNativesUSchemaDatabase()
	{
	}
	UClass* Z_Construct_UClass_USchemaDatabase_NoRegister()
	{
		return USchemaDatabase::StaticClass();
	}
	struct Z_Construct_UClass_USchemaDatabase_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NextAvailableComponentId_MetaData[];
#endif
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_NextAvailableComponentId;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LevelComponentIds_MetaData[];
#endif
		static const UE4CodeGen_Private::FSetPropertyParams NewProp_LevelComponentIds;
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_LevelComponentIds_ElementProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ComponentIdToClassPath_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_ComponentIdToClassPath;
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_ComponentIdToClassPath_Key_KeyProp;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ComponentIdToClassPath_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LevelPathToComponentId_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_LevelPathToComponentId;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_LevelPathToComponentId_Key_KeyProp;
		static const UE4CodeGen_Private::FUInt32PropertyParams NewProp_LevelPathToComponentId_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SubobjectClassPathToSchema_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_SubobjectClassPathToSchema;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_SubobjectClassPathToSchema_Key_KeyProp;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_SubobjectClassPathToSchema_ValueProp;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClassPathToSchema_MetaData[];
#endif
		static const UE4CodeGen_Private::FMapPropertyParams NewProp_ActorClassPathToSchema;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ActorClassPathToSchema_Key_KeyProp;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_ActorClassPathToSchema_ValueProp;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USchemaDatabase_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UDataAsset,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Utils/SchemaDatabase.h" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_NextAvailableComponentId_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_NextAvailableComponentId = { "NextAvailableComponentId", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, NextAvailableComponentId), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_NextAvailableComponentId_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_NextAvailableComponentId_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FSetPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds = { "LevelComponentIds", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Set, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, LevelComponentIds), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds_MetaData)) };
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds_ElementProp = { "LevelComponentIds", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath = { "ComponentIdToClassPath", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, ComponentIdToClassPath), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_MetaData)) };
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_Key_KeyProp = { "ComponentIdToClassPath_Key", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_ValueProp = { "ComponentIdToClassPath", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId = { "LevelPathToComponentId", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, LevelPathToComponentId), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_Key_KeyProp = { "LevelPathToComponentId_Key", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FUInt32PropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_ValueProp = { "LevelPathToComponentId", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::UInt32, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema = { "SubobjectClassPathToSchema", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, SubobjectClassPathToSchema), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_Key_KeyProp = { "SubobjectClassPathToSchema_Key", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_ValueProp = { "SubobjectClassPathToSchema", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, Z_Construct_UScriptStruct_FSubobjectSchemaData, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_MetaData[] = {
		{ "Category", "SpatialGDK" },
		{ "ModuleRelativePath", "Public/Utils/SchemaDatabase.h" },
	};
#endif
	const UE4CodeGen_Private::FMapPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema = { "ActorClassPathToSchema", nullptr, (EPropertyFlags)0x0010000000020001, UE4CodeGen_Private::EPropertyGenFlags::Map, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USchemaDatabase, ActorClassPathToSchema), METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_MetaData, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_MetaData)) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_Key_KeyProp = { "ActorClassPathToSchema_Key", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_ValueProp = { "ActorClassPathToSchema", nullptr, (EPropertyFlags)0x0000000000020001, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 1, Z_Construct_UScriptStruct_FActorSchemaData, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USchemaDatabase_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_NextAvailableComponentId,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelComponentIds_ElementProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ComponentIdToClassPath_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_LevelPathToComponentId_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_SubobjectClassPathToSchema_ValueProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_Key_KeyProp,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USchemaDatabase_Statics::NewProp_ActorClassPathToSchema_ValueProp,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USchemaDatabase_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USchemaDatabase>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USchemaDatabase_Statics::ClassParams = {
		&USchemaDatabase::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USchemaDatabase_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::PropPointers),
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USchemaDatabase_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USchemaDatabase_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USchemaDatabase()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USchemaDatabase_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USchemaDatabase, 170410984);
	template<> SPATIALGDK_API UClass* StaticClass<USchemaDatabase>()
	{
		return USchemaDatabase::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USchemaDatabase(Z_Construct_UClass_USchemaDatabase, &USchemaDatabase::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USchemaDatabase"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USchemaDatabase);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
