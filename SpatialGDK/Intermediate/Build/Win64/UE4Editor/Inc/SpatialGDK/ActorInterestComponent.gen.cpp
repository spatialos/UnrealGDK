// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/Components/ActorInterestComponent.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeActorInterestComponent() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorInterestComponent_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorInterestComponent();
	ENGINE_API UClass* Z_Construct_UClass_UActorComponent();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FQueryData();
// End Cross Module References
	void UActorInterestComponent::StaticRegisterNativesUActorInterestComponent()
	{
	}
	UClass* Z_Construct_UClass_UActorInterestComponent_NoRegister()
	{
		return UActorInterestComponent::StaticClass();
	}
	struct Z_Construct_UClass_UActorInterestComponent_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Queries_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Queries;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_Queries_Inner;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_MetaData[];
#endif
		static void NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bUseNetCullDistanceSquaredForCheckoutRadius;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UActorInterestComponent_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UActorComponent,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorInterestComponent_Statics::Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "SpatialGDK" },
		{ "IncludePath", "EngineClasses/Components/ActorInterestComponent.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/Components/ActorInterestComponent.h" },
		{ "ToolTip", "Creates a set of SpatialOS Queries for describing interest that this actor has in other entities." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries_MetaData[] = {
		{ "Category", "Interest" },
		{ "ModuleRelativePath", "Public/EngineClasses/Components/ActorInterestComponent.h" },
		{ "ToolTip", "The Queries associated with this component." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries = { "Queries", nullptr, (EPropertyFlags)0x0010008000010015, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UActorInterestComponent, Queries), METADATA_PARAMS(Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries_MetaData, ARRAY_COUNT(Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries_MetaData)) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries_Inner = { "Queries", nullptr, (EPropertyFlags)0x0000008000000000, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FQueryData, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_MetaData[] = {
		{ "Category", "Interest" },
		{ "ModuleRelativePath", "Public/EngineClasses/Components/ActorInterestComponent.h" },
		{ "ToolTip", "Whether to use NetCullDistanceSquared to generate constraints relative to the Actor that this component is attached to." },
	};
#endif
	void Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_SetBit(void* Obj)
	{
		((UActorInterestComponent*)Obj)->bUseNetCullDistanceSquaredForCheckoutRadius = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius = { "bUseNetCullDistanceSquaredForCheckoutRadius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UActorInterestComponent), &Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_SetBit, METADATA_PARAMS(Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_MetaData, ARRAY_COUNT(Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UActorInterestComponent_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_Queries_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UActorInterestComponent_Statics::NewProp_bUseNetCullDistanceSquaredForCheckoutRadius,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UActorInterestComponent_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UActorInterestComponent>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UActorInterestComponent_Statics::ClassParams = {
		&UActorInterestComponent::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UActorInterestComponent_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UActorInterestComponent_Statics::PropPointers),
		0,
		0x00B000A4u,
		0x00000008u,
		METADATA_PARAMS(Z_Construct_UClass_UActorInterestComponent_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UActorInterestComponent_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UActorInterestComponent()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UActorInterestComponent_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UActorInterestComponent, 1595087913);
	template<> SPATIALGDK_API UClass* StaticClass<UActorInterestComponent>()
	{
		return UActorInterestComponent::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UActorInterestComponent(Z_Construct_UClass_UActorInterestComponent, &UActorInterestComponent::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UActorInterestComponent"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UActorInterestComponent);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
