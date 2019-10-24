// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDKEditorCommandlet/Private/Commandlets/GenerateSnapshotCommandlet.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeGenerateSnapshotCommandlet() {}
// Cross Module References
	SPATIALGDKEDITORCOMMANDLET_API UClass* Z_Construct_UClass_UGenerateSnapshotCommandlet_NoRegister();
	SPATIALGDKEDITORCOMMANDLET_API UClass* Z_Construct_UClass_UGenerateSnapshotCommandlet();
	ENGINE_API UClass* Z_Construct_UClass_UCommandlet();
	UPackage* Z_Construct_UPackage__Script_SpatialGDKEditorCommandlet();
// End Cross Module References
	void UGenerateSnapshotCommandlet::StaticRegisterNativesUGenerateSnapshotCommandlet()
	{
	}
	UClass* Z_Construct_UClass_UGenerateSnapshotCommandlet_NoRegister()
	{
		return UGenerateSnapshotCommandlet::StaticClass();
	}
	struct Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UCommandlet,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDKEditorCommandlet,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Commandlets/GenerateSnapshotCommandlet.h" },
		{ "ModuleRelativePath", "Private/Commandlets/GenerateSnapshotCommandlet.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UGenerateSnapshotCommandlet>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::ClassParams = {
		&UGenerateSnapshotCommandlet::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UGenerateSnapshotCommandlet()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UGenerateSnapshotCommandlet_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UGenerateSnapshotCommandlet, 3293460474);
	template<> SPATIALGDKEDITORCOMMANDLET_API UClass* StaticClass<UGenerateSnapshotCommandlet>()
	{
		return UGenerateSnapshotCommandlet::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UGenerateSnapshotCommandlet(Z_Construct_UClass_UGenerateSnapshotCommandlet, &UGenerateSnapshotCommandlet::StaticClass, TEXT("/Script/SpatialGDKEditorCommandlet"), TEXT("UGenerateSnapshotCommandlet"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UGenerateSnapshotCommandlet);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
