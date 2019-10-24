// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Utils/SnapshotGenerationTemplate.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSnapshotGenerationTemplate() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USnapshotGenerationTemplate_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USnapshotGenerationTemplate();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
// End Cross Module References
	void USnapshotGenerationTemplate::StaticRegisterNativesUSnapshotGenerationTemplate()
	{
	}
	UClass* Z_Construct_UClass_USnapshotGenerationTemplate_NoRegister()
	{
		return USnapshotGenerationTemplate::StaticClass();
	}
	struct Z_Construct_UClass_USnapshotGenerationTemplate_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USnapshotGenerationTemplate_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USnapshotGenerationTemplate_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Utils/SnapshotGenerationTemplate.h" },
		{ "ModuleRelativePath", "Public/Utils/SnapshotGenerationTemplate.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USnapshotGenerationTemplate_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USnapshotGenerationTemplate>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USnapshotGenerationTemplate_Statics::ClassParams = {
		&USnapshotGenerationTemplate::StaticClass,
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
		0x001000A1u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USnapshotGenerationTemplate_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USnapshotGenerationTemplate_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USnapshotGenerationTemplate()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USnapshotGenerationTemplate_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USnapshotGenerationTemplate, 1781490261);
	template<> SPATIALGDK_API UClass* StaticClass<USnapshotGenerationTemplate>()
	{
		return USnapshotGenerationTemplate::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USnapshotGenerationTemplate(Z_Construct_UClass_USnapshotGenerationTemplate, &USnapshotGenerationTemplate::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USnapshotGenerationTemplate"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USnapshotGenerationTemplate);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
