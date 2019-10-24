// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/SpatialStaticComponentView.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialStaticComponentView() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStaticComponentView_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStaticComponentView();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
// End Cross Module References
	void USpatialStaticComponentView::StaticRegisterNativesUSpatialStaticComponentView()
	{
	}
	UClass* Z_Construct_UClass_USpatialStaticComponentView_NoRegister()
	{
		return USpatialStaticComponentView::StaticClass();
	}
	struct Z_Construct_UClass_USpatialStaticComponentView_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialStaticComponentView_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialStaticComponentView_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Interop/SpatialStaticComponentView.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialStaticComponentView.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialStaticComponentView_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialStaticComponentView>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialStaticComponentView_Statics::ClassParams = {
		&USpatialStaticComponentView::StaticClass,
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
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialStaticComponentView_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialStaticComponentView_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialStaticComponentView()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialStaticComponentView_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialStaticComponentView, 4197969133);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialStaticComponentView>()
	{
		return USpatialStaticComponentView::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialStaticComponentView(Z_Construct_UClass_USpatialStaticComponentView, &USpatialStaticComponentView::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialStaticComponentView"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialStaticComponentView);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
