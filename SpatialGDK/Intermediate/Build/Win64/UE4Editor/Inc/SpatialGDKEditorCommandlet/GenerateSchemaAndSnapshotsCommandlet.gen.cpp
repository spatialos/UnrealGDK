// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDKEditorCommandlet/Private/Commandlets/GenerateSchemaAndSnapshotsCommandlet.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeGenerateSchemaAndSnapshotsCommandlet() {}
// Cross Module References
	SPATIALGDKEDITORCOMMANDLET_API UClass* Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_NoRegister();
	SPATIALGDKEDITORCOMMANDLET_API UClass* Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet();
	ENGINE_API UClass* Z_Construct_UClass_UCommandlet();
	UPackage* Z_Construct_UPackage__Script_SpatialGDKEditorCommandlet();
// End Cross Module References
	void UGenerateSchemaAndSnapshotsCommandlet::StaticRegisterNativesUGenerateSchemaAndSnapshotsCommandlet()
	{
	}
	UClass* Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_NoRegister()
	{
		return UGenerateSchemaAndSnapshotsCommandlet::StaticClass();
	}
	struct Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UCommandlet,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditorCommandlet,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Commandlets/GenerateSchemaAndSnapshotsCommandlet.h" },
		{ "ModuleRelativePath", "Private/Commandlets/GenerateSchemaAndSnapshotsCommandlet.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UGenerateSchemaAndSnapshotsCommandlet>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::ClassParams = {
		&UGenerateSchemaAndSnapshotsCommandlet::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x000000A8u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UGenerateSchemaAndSnapshotsCommandlet, 1098494146);
	template<> SPATIALGDKEDITORCOMMANDLET_API UClass* StaticClass<UGenerateSchemaAndSnapshotsCommandlet>()
	{
		return UGenerateSchemaAndSnapshotsCommandlet::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UGenerateSchemaAndSnapshotsCommandlet(Z_Construct_UClass_UGenerateSchemaAndSnapshotsCommandlet, &UGenerateSchemaAndSnapshotsCommandlet::StaticClass, TEXT("/Script/SpatialGDKEditorCommandlet"), TEXT("UGenerateSchemaAndSnapshotsCommandlet"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UGenerateSchemaAndSnapshotsCommandlet);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
