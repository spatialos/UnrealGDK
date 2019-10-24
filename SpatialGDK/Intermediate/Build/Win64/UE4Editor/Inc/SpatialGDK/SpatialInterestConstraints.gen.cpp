// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/SpatialInterestConstraints.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialInterestConstraints() {}
// Cross Module References
	SPATIALGDK_API UScriptStruct* Z_Construct_UScriptStruct_FQueryData();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_UAbstractQueryConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UAbstractQueryConstraint();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	SPATIALGDK_API UClass* Z_Construct_UClass_UOrConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UOrConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_UAndConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UAndConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_USphereConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USphereConstraint();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector();
	SPATIALGDK_API UClass* Z_Construct_UClass_UCylinderConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UCylinderConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_UBoxConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UBoxConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeSphereConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeSphereConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeCylinderConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeCylinderConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeBoxConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_URelativeBoxConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_UCheckoutRadiusConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UCheckoutRadiusConstraint();
	COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
	ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorClassConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UActorClassConstraint();
	SPATIALGDK_API UClass* Z_Construct_UClass_UComponentClassConstraint_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_UComponentClassConstraint();
	ENGINE_API UClass* Z_Construct_UClass_UActorComponent_NoRegister();
// End Cross Module References
class UScriptStruct* FQueryData::StaticStruct()
{
	static class UScriptStruct* Singleton = NULL;
	if (!Singleton)
	{
		extern SPATIALGDK_API uint32 Get_Z_Construct_UScriptStruct_FQueryData_Hash();
		Singleton = GetStaticStruct(Z_Construct_UScriptStruct_FQueryData, Z_Construct_UPackage__Script_SpatialGDK(), TEXT("QueryData"), sizeof(FQueryData), Get_Z_Construct_UScriptStruct_FQueryData_Hash());
	}
	return Singleton;
}
template<> SPATIALGDK_API UScriptStruct* StaticStruct<FQueryData>()
{
	return FQueryData::StaticStruct();
}
static FCompiledInDeferStruct Z_CompiledInDeferStruct_UScriptStruct_FQueryData(FQueryData::StaticStruct, TEXT("/Script/SpatialGDK"), TEXT("QueryData"), false, nullptr, nullptr);
static struct FScriptStruct_SpatialGDK_StaticRegisterNativesFQueryData
{
	FScriptStruct_SpatialGDK_StaticRegisterNativesFQueryData()
	{
		UScriptStruct::DeferCppStructOps(FName(TEXT("QueryData")),new UScriptStruct::TCppStructOps<FQueryData>);
	}
} ScriptStruct_SpatialGDK_StaticRegisterNativesFQueryData;
	struct Z_Construct_UScriptStruct_FQueryData_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Frequency_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Frequency;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Constraint_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Constraint;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UE4CodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FQueryData_Statics::Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Query data used to configure Query-based Interest." },
	};
#endif
	void* Z_Construct_UScriptStruct_FQueryData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FQueryData>();
	}
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Frequency_MetaData[] = {
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Not currently supported.\n\nUsed for frequency-based rate limiting. Represents the maximum frequency\nof updates for this particular query. An empty option represents no\nrate-limiting (ie. updates are received as soon as possible). Frequency\nis measured in Hz.\n\nIf set, the time between consecutive updates will be at least\n1/frequency. This is determined at the time that updates are sent from\nthe Runtime and may not necessarily correspond to the time updates are\nreceived by the worker.\n\nIf after an update has been sent, multiple updates are applied to a\ncomponent, they will be merged and sent as a single update after\n1/frequency of the last sent update. When components with events are\nmerged, the resultant component will contain a concatenation of all the\nevents.\n\nIf multiple queries match the same Entity-Component then the highest of\nall frequencies is used." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Frequency = { "Frequency", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FQueryData, Frequency), METADATA_PARAMS(Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Frequency_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Frequency_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Constraint_MetaData[] = {
		{ "Category", "Query Data" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The root constraint associated with the query generated by this component." },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Constraint = { "Constraint", nullptr, (EPropertyFlags)0x001200000009001d, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FQueryData, Constraint), Z_Construct_UClass_UAbstractQueryConstraint_NoRegister, METADATA_PARAMS(Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Constraint_MetaData, ARRAY_COUNT(Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Constraint_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FQueryData_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Frequency,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FQueryData_Statics::NewProp_Constraint,
	};
	const UE4CodeGen_Private::FStructParams Z_Construct_UScriptStruct_FQueryData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
		nullptr,
		&NewStructOps,
		"QueryData",
		sizeof(FQueryData),
		alignof(FQueryData),
		Z_Construct_UScriptStruct_FQueryData_Statics::PropPointers,
		ARRAY_COUNT(Z_Construct_UScriptStruct_FQueryData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000205),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FQueryData_Statics::Struct_MetaDataParams, ARRAY_COUNT(Z_Construct_UScriptStruct_FQueryData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FQueryData()
	{
#if WITH_HOT_RELOAD
		extern uint32 Get_Z_Construct_UScriptStruct_FQueryData_Hash();
		UPackage* Outer = Z_Construct_UPackage__Script_SpatialGDK();
		static UScriptStruct* ReturnStruct = FindExistingStructIfHotReloadOrDynamic(Outer, TEXT("QueryData"), sizeof(FQueryData), Get_Z_Construct_UScriptStruct_FQueryData_Hash(), false);
#else
		static UScriptStruct* ReturnStruct = nullptr;
#endif
		if (!ReturnStruct)
		{
			UE4CodeGen_Private::ConstructUScriptStruct(ReturnStruct, Z_Construct_UScriptStruct_FQueryData_Statics::ReturnStructParams);
		}
		return ReturnStruct;
	}
	uint32 Get_Z_Construct_UScriptStruct_FQueryData_Hash() { return 3203627911U; }
	void UAbstractQueryConstraint::StaticRegisterNativesUAbstractQueryConstraint()
	{
	}
	UClass* Z_Construct_UClass_UAbstractQueryConstraint_NoRegister()
	{
		return UAbstractQueryConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UAbstractQueryConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UAbstractQueryConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UAbstractQueryConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintInternalUseOnly", "true" },
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_UAbstractQueryConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UAbstractQueryConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UAbstractQueryConstraint_Statics::ClassParams = {
		&UAbstractQueryConstraint::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_UAbstractQueryConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UAbstractQueryConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UAbstractQueryConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UAbstractQueryConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UAbstractQueryConstraint, 3148408679);
	template<> SPATIALGDK_API UClass* StaticClass<UAbstractQueryConstraint>()
	{
		return UAbstractQueryConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UAbstractQueryConstraint(Z_Construct_UClass_UAbstractQueryConstraint, &UAbstractQueryConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UAbstractQueryConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UAbstractQueryConstraint);
	void UOrConstraint::StaticRegisterNativesUOrConstraint()
	{
	}
	UClass* Z_Construct_UClass_UOrConstraint_NoRegister()
	{
		return UOrConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UOrConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Constraints_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Constraints;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Constraints_Inner_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Constraints_Inner;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UOrConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UOrConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that is satisfied if any of its inner constraints are satisfied." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_MetaData[] = {
		{ "Category", "Or Constraint" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Entities captured by any subconstraints will be included in interest results." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints = { "Constraints", nullptr, (EPropertyFlags)0x001000800001001d, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UOrConstraint, Constraints), METADATA_PARAMS(Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_MetaData, ARRAY_COUNT(Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_Inner_MetaData[] = {
		{ "Category", "Or Constraint" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Entities captured by any subconstraints will be included in interest results." },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_Inner = { "Constraints", nullptr, (EPropertyFlags)0x0002000000080008, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UClass_UAbstractQueryConstraint_NoRegister, METADATA_PARAMS(Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_Inner_MetaData, ARRAY_COUNT(Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_Inner_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UOrConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UOrConstraint_Statics::NewProp_Constraints_Inner,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UOrConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UOrConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UOrConstraint_Statics::ClassParams = {
		&UOrConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UOrConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UOrConstraint_Statics::PropPointers),
		0,
		0x00B010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UOrConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UOrConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UOrConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UOrConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UOrConstraint, 3703622423);
	template<> SPATIALGDK_API UClass* StaticClass<UOrConstraint>()
	{
		return UOrConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UOrConstraint(Z_Construct_UClass_UOrConstraint, &UOrConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UOrConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UOrConstraint);
	void UAndConstraint::StaticRegisterNativesUAndConstraint()
	{
	}
	UClass* Z_Construct_UClass_UAndConstraint_NoRegister()
	{
		return UAndConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UAndConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Constraints_MetaData[];
#endif
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Constraints;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Constraints_Inner_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Constraints_Inner;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UAndConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UAndConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that is satisfied if all of its inner constraints are satisfied." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_MetaData[] = {
		{ "Category", "And Constraint" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Entities captured by all subconstraints will be included in interest results." },
	};
#endif
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints = { "Constraints", nullptr, (EPropertyFlags)0x001000800001001d, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UAndConstraint, Constraints), METADATA_PARAMS(Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_MetaData, ARRAY_COUNT(Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_Inner_MetaData[] = {
		{ "Category", "And Constraint" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Entities captured by all subconstraints will be included in interest results." },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_Inner = { "Constraints", nullptr, (EPropertyFlags)0x0002000000080008, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UClass_UAbstractQueryConstraint_NoRegister, METADATA_PARAMS(Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_Inner_MetaData, ARRAY_COUNT(Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_Inner_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UAndConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UAndConstraint_Statics::NewProp_Constraints_Inner,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UAndConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UAndConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UAndConstraint_Statics::ClassParams = {
		&UAndConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UAndConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UAndConstraint_Statics::PropPointers),
		0,
		0x00B010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UAndConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UAndConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UAndConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UAndConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UAndConstraint, 175698931);
	template<> SPATIALGDK_API UClass* StaticClass<UAndConstraint>()
	{
		return UAndConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UAndConstraint(Z_Construct_UClass_UAndConstraint, &UAndConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UAndConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UAndConstraint);
	void USphereConstraint::StaticRegisterNativesUSphereConstraint()
	{
	}
	UClass* Z_Construct_UClass_USphereConstraint_NoRegister()
	{
		return USphereConstraint::StaticClass();
	}
	struct Z_Construct_UClass_USphereConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Radius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Radius;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Center_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_Center;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USphereConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USphereConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a sphere centered on the specified point." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USphereConstraint_Statics::NewProp_Radius_MetaData[] = {
		{ "Category", "Sphere Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the sphere represented by this constraint in centimeters." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_USphereConstraint_Statics::NewProp_Radius = { "Radius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USphereConstraint, Radius), METADATA_PARAMS(Z_Construct_UClass_USphereConstraint_Statics::NewProp_Radius_MetaData, ARRAY_COUNT(Z_Construct_UClass_USphereConstraint_Statics::NewProp_Radius_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USphereConstraint_Statics::NewProp_Center_MetaData[] = {
		{ "Category", "Sphere Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The location in the world that this constraint is relative to." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_USphereConstraint_Statics::NewProp_Center = { "Center", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USphereConstraint, Center), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(Z_Construct_UClass_USphereConstraint_Statics::NewProp_Center_MetaData, ARRAY_COUNT(Z_Construct_UClass_USphereConstraint_Statics::NewProp_Center_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USphereConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USphereConstraint_Statics::NewProp_Radius,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USphereConstraint_Statics::NewProp_Center,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USphereConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USphereConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USphereConstraint_Statics::ClassParams = {
		&USphereConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USphereConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USphereConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USphereConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USphereConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USphereConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USphereConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USphereConstraint, 3993781011);
	template<> SPATIALGDK_API UClass* StaticClass<USphereConstraint>()
	{
		return USphereConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USphereConstraint(Z_Construct_UClass_USphereConstraint, &USphereConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USphereConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USphereConstraint);
	void UCylinderConstraint::StaticRegisterNativesUCylinderConstraint()
	{
	}
	UClass* Z_Construct_UClass_UCylinderConstraint_NoRegister()
	{
		return UCylinderConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UCylinderConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Radius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Radius;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Center_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_Center;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UCylinderConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCylinderConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a cylinder centered on the specified point." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Radius_MetaData[] = {
		{ "Category", "Cylinder Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the cylinder represented by this constraint in centimeters." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Radius = { "Radius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UCylinderConstraint, Radius), METADATA_PARAMS(Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Radius_MetaData, ARRAY_COUNT(Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Radius_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Center_MetaData[] = {
		{ "Category", "Cylinder Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The location in the world that this constraint is relative to." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Center = { "Center", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UCylinderConstraint, Center), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Center_MetaData, ARRAY_COUNT(Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Center_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UCylinderConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Radius,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCylinderConstraint_Statics::NewProp_Center,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UCylinderConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UCylinderConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UCylinderConstraint_Statics::ClassParams = {
		&UCylinderConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UCylinderConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UCylinderConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UCylinderConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UCylinderConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UCylinderConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UCylinderConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UCylinderConstraint, 1426070349);
	template<> SPATIALGDK_API UClass* StaticClass<UCylinderConstraint>()
	{
		return UCylinderConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UCylinderConstraint(Z_Construct_UClass_UCylinderConstraint, &UCylinderConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UCylinderConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UCylinderConstraint);
	void UBoxConstraint::StaticRegisterNativesUBoxConstraint()
	{
	}
	UClass* Z_Construct_UClass_UBoxConstraint_NoRegister()
	{
		return UBoxConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UBoxConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EdgeLengths_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_EdgeLengths;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Center_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_Center;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UBoxConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UBoxConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a bounding box centered on the specified point." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UBoxConstraint_Statics::NewProp_EdgeLengths_MetaData[] = {
		{ "Category", "Box Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the box represented by this constraint." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_UBoxConstraint_Statics::NewProp_EdgeLengths = { "EdgeLengths", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UBoxConstraint, EdgeLengths), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(Z_Construct_UClass_UBoxConstraint_Statics::NewProp_EdgeLengths_MetaData, ARRAY_COUNT(Z_Construct_UClass_UBoxConstraint_Statics::NewProp_EdgeLengths_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UBoxConstraint_Statics::NewProp_Center_MetaData[] = {
		{ "Category", "Box Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The location in the world that this constraint is relative to." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_UBoxConstraint_Statics::NewProp_Center = { "Center", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UBoxConstraint, Center), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(Z_Construct_UClass_UBoxConstraint_Statics::NewProp_Center_MetaData, ARRAY_COUNT(Z_Construct_UClass_UBoxConstraint_Statics::NewProp_Center_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UBoxConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UBoxConstraint_Statics::NewProp_EdgeLengths,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UBoxConstraint_Statics::NewProp_Center,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UBoxConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UBoxConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UBoxConstraint_Statics::ClassParams = {
		&UBoxConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UBoxConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UBoxConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UBoxConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UBoxConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UBoxConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UBoxConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UBoxConstraint, 2798314050);
	template<> SPATIALGDK_API UClass* StaticClass<UBoxConstraint>()
	{
		return UBoxConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UBoxConstraint(Z_Construct_UClass_UBoxConstraint, &UBoxConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UBoxConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UBoxConstraint);
	void URelativeSphereConstraint::StaticRegisterNativesURelativeSphereConstraint()
	{
	}
	UClass* Z_Construct_UClass_URelativeSphereConstraint_NoRegister()
	{
		return URelativeSphereConstraint::StaticClass();
	}
	struct Z_Construct_UClass_URelativeSphereConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Radius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Radius;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_URelativeSphereConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeSphereConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a sphere centered on the actor." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeSphereConstraint_Statics::NewProp_Radius_MetaData[] = {
		{ "Category", "Relative Sphere Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the sphere represented by this constraint in centimeters." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_URelativeSphereConstraint_Statics::NewProp_Radius = { "Radius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(URelativeSphereConstraint, Radius), METADATA_PARAMS(Z_Construct_UClass_URelativeSphereConstraint_Statics::NewProp_Radius_MetaData, ARRAY_COUNT(Z_Construct_UClass_URelativeSphereConstraint_Statics::NewProp_Radius_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_URelativeSphereConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_URelativeSphereConstraint_Statics::NewProp_Radius,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_URelativeSphereConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<URelativeSphereConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_URelativeSphereConstraint_Statics::ClassParams = {
		&URelativeSphereConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_URelativeSphereConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_URelativeSphereConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_URelativeSphereConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_URelativeSphereConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_URelativeSphereConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_URelativeSphereConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(URelativeSphereConstraint, 228155303);
	template<> SPATIALGDK_API UClass* StaticClass<URelativeSphereConstraint>()
	{
		return URelativeSphereConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_URelativeSphereConstraint(Z_Construct_UClass_URelativeSphereConstraint, &URelativeSphereConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("URelativeSphereConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(URelativeSphereConstraint);
	void URelativeCylinderConstraint::StaticRegisterNativesURelativeCylinderConstraint()
	{
	}
	UClass* Z_Construct_UClass_URelativeCylinderConstraint_NoRegister()
	{
		return URelativeCylinderConstraint::StaticClass();
	}
	struct Z_Construct_UClass_URelativeCylinderConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Radius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Radius;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_URelativeCylinderConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeCylinderConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a cylinder centered on the actor." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeCylinderConstraint_Statics::NewProp_Radius_MetaData[] = {
		{ "Category", "Relative Cylinder Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the cylinder represented by this constraint in centimeters." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_URelativeCylinderConstraint_Statics::NewProp_Radius = { "Radius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(URelativeCylinderConstraint, Radius), METADATA_PARAMS(Z_Construct_UClass_URelativeCylinderConstraint_Statics::NewProp_Radius_MetaData, ARRAY_COUNT(Z_Construct_UClass_URelativeCylinderConstraint_Statics::NewProp_Radius_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_URelativeCylinderConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_URelativeCylinderConstraint_Statics::NewProp_Radius,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_URelativeCylinderConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<URelativeCylinderConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_URelativeCylinderConstraint_Statics::ClassParams = {
		&URelativeCylinderConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_URelativeCylinderConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_URelativeCylinderConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_URelativeCylinderConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_URelativeCylinderConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_URelativeCylinderConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_URelativeCylinderConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(URelativeCylinderConstraint, 4077770969);
	template<> SPATIALGDK_API UClass* StaticClass<URelativeCylinderConstraint>()
	{
		return URelativeCylinderConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_URelativeCylinderConstraint(Z_Construct_UClass_URelativeCylinderConstraint, &URelativeCylinderConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("URelativeCylinderConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(URelativeCylinderConstraint);
	void URelativeBoxConstraint::StaticRegisterNativesURelativeBoxConstraint()
	{
	}
	UClass* Z_Construct_UClass_URelativeBoxConstraint_NoRegister()
	{
		return URelativeBoxConstraint::StaticClass();
	}
	struct Z_Construct_UClass_URelativeBoxConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_EdgeLengths_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_EdgeLengths;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_URelativeBoxConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeBoxConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all entities within a bounding box centered on the actor." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URelativeBoxConstraint_Statics::NewProp_EdgeLengths_MetaData[] = {
		{ "Category", "Relative Box Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the box represented by this constraint." },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_URelativeBoxConstraint_Statics::NewProp_EdgeLengths = { "EdgeLengths", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(URelativeBoxConstraint, EdgeLengths), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(Z_Construct_UClass_URelativeBoxConstraint_Statics::NewProp_EdgeLengths_MetaData, ARRAY_COUNT(Z_Construct_UClass_URelativeBoxConstraint_Statics::NewProp_EdgeLengths_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_URelativeBoxConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_URelativeBoxConstraint_Statics::NewProp_EdgeLengths,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_URelativeBoxConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<URelativeBoxConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_URelativeBoxConstraint_Statics::ClassParams = {
		&URelativeBoxConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_URelativeBoxConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_URelativeBoxConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_URelativeBoxConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_URelativeBoxConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_URelativeBoxConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_URelativeBoxConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(URelativeBoxConstraint, 3780913557);
	template<> SPATIALGDK_API UClass* StaticClass<URelativeBoxConstraint>()
	{
		return URelativeBoxConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_URelativeBoxConstraint(Z_Construct_UClass_URelativeBoxConstraint, &URelativeBoxConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("URelativeBoxConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(URelativeBoxConstraint);
	void UCheckoutRadiusConstraint::StaticRegisterNativesUCheckoutRadiusConstraint()
	{
	}
	UClass* Z_Construct_UClass_UCheckoutRadiusConstraint_NoRegister()
	{
		return UCheckoutRadiusConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UCheckoutRadiusConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Radius_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Radius;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FClassPropertyParams NewProp_ActorClass;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes an actor type (including subtypes) and a cylindrical range around the actor with the interest." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_Radius_MetaData[] = {
		{ "Category", "Checkout Radius Constraint" },
		{ "ClampMin", "0.000000" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The size of the cylinder represented by this constraint in centimeters." },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_Radius = { "Radius", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UCheckoutRadiusConstraint, Radius), METADATA_PARAMS(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_Radius_MetaData, ARRAY_COUNT(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_Radius_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_ActorClass_MetaData[] = {
		{ "Category", "Checkout Radius Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The base type of actor that this constraint will capture." },
	};
#endif
	const UE4CodeGen_Private::FClassPropertyParams Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_ActorClass = { "ActorClass", nullptr, (EPropertyFlags)0x0014000000010015, UE4CodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UCheckoutRadiusConstraint, ActorClass), Z_Construct_UClass_AActor_NoRegister, Z_Construct_UClass_UClass, METADATA_PARAMS(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_ActorClass_MetaData, ARRAY_COUNT(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_ActorClass_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_Radius,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::NewProp_ActorClass,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UCheckoutRadiusConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::ClassParams = {
		&UCheckoutRadiusConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UCheckoutRadiusConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UCheckoutRadiusConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UCheckoutRadiusConstraint, 3448763526);
	template<> SPATIALGDK_API UClass* StaticClass<UCheckoutRadiusConstraint>()
	{
		return UCheckoutRadiusConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UCheckoutRadiusConstraint(Z_Construct_UClass_UCheckoutRadiusConstraint, &UCheckoutRadiusConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UCheckoutRadiusConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UCheckoutRadiusConstraint);
	void UActorClassConstraint::StaticRegisterNativesUActorClassConstraint()
	{
	}
	UClass* Z_Construct_UClass_UActorClassConstraint_NoRegister()
	{
		return UActorClassConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UActorClassConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bIncludeDerivedClasses_MetaData[];
#endif
		static void NewProp_bIncludeDerivedClasses_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bIncludeDerivedClasses;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FClassPropertyParams NewProp_ActorClass;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UActorClassConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorClassConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all actors of a type (optionally including subtypes)." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData[] = {
		{ "Category", "Actor Class Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Whether this constraint should capture derived types." },
	};
#endif
	void Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses_SetBit(void* Obj)
	{
		((UActorClassConstraint*)Obj)->bIncludeDerivedClasses = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses = { "bIncludeDerivedClasses", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UActorClassConstraint), &Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses_SetBit, METADATA_PARAMS(Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData, ARRAY_COUNT(Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_ActorClass_MetaData[] = {
		{ "Category", "Actor Class Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The base type of actor that this constraint will capture." },
	};
#endif
	const UE4CodeGen_Private::FClassPropertyParams Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_ActorClass = { "ActorClass", nullptr, (EPropertyFlags)0x0014000000010015, UE4CodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UActorClassConstraint, ActorClass), Z_Construct_UClass_AActor_NoRegister, Z_Construct_UClass_UClass, METADATA_PARAMS(Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_ActorClass_MetaData, ARRAY_COUNT(Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_ActorClass_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UActorClassConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_bIncludeDerivedClasses,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UActorClassConstraint_Statics::NewProp_ActorClass,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UActorClassConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UActorClassConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UActorClassConstraint_Statics::ClassParams = {
		&UActorClassConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UActorClassConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UActorClassConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UActorClassConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UActorClassConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UActorClassConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UActorClassConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UActorClassConstraint, 847102047);
	template<> SPATIALGDK_API UClass* StaticClass<UActorClassConstraint>()
	{
		return UActorClassConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UActorClassConstraint(Z_Construct_UClass_UActorClassConstraint, &UActorClassConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UActorClassConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UActorClassConstraint);
	void UComponentClassConstraint::StaticRegisterNativesUComponentClassConstraint()
	{
	}
	UClass* Z_Construct_UClass_UComponentClassConstraint_NoRegister()
	{
		return UComponentClassConstraint::StaticClass();
	}
	struct Z_Construct_UClass_UComponentClassConstraint_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bIncludeDerivedClasses_MetaData[];
#endif
		static void NewProp_bIncludeDerivedClasses_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bIncludeDerivedClasses;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ComponentClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FClassPropertyParams NewProp_ComponentClass;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_UComponentClassConstraint_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UAbstractQueryConstraint,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UComponentClassConstraint_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "IncludePath", "Interop/SpatialInterestConstraints.h" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Creates a constraint that includes all components of a type (optionally including subtypes)." },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData[] = {
		{ "Category", "Component Class Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "Whether this constraint should capture derived types." },
	};
#endif
	void Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses_SetBit(void* Obj)
	{
		((UComponentClassConstraint*)Obj)->bIncludeDerivedClasses = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses = { "bIncludeDerivedClasses", nullptr, (EPropertyFlags)0x0010000000010015, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(UComponentClassConstraint), &Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses_SetBit, METADATA_PARAMS(Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData, ARRAY_COUNT(Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_ComponentClass_MetaData[] = {
		{ "Category", "Component Class Constraint" },
		{ "ModuleRelativePath", "Public/Interop/SpatialInterestConstraints.h" },
		{ "ToolTip", "The base type of component that this constraint will capture." },
	};
#endif
	const UE4CodeGen_Private::FClassPropertyParams Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_ComponentClass = { "ComponentClass", nullptr, (EPropertyFlags)0x0014000000010015, UE4CodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(UComponentClassConstraint, ComponentClass), Z_Construct_UClass_UActorComponent_NoRegister, Z_Construct_UClass_UClass, METADATA_PARAMS(Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_ComponentClass_MetaData, ARRAY_COUNT(Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_ComponentClass_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UComponentClassConstraint_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_bIncludeDerivedClasses,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UComponentClassConstraint_Statics::NewProp_ComponentClass,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_UComponentClassConstraint_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UComponentClassConstraint>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_UComponentClassConstraint_Statics::ClassParams = {
		&UComponentClassConstraint::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_UComponentClassConstraint_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_UComponentClassConstraint_Statics::PropPointers),
		0,
		0x003010A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_UComponentClassConstraint_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_UComponentClassConstraint_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_UComponentClassConstraint()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_UComponentClassConstraint_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(UComponentClassConstraint, 1794588178);
	template<> SPATIALGDK_API UClass* StaticClass<UComponentClassConstraint>()
	{
		return UComponentClassConstraint::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_UComponentClassConstraint(Z_Construct_UClass_UComponentClassConstraint, &UComponentClassConstraint::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("UComponentClassConstraint"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(UComponentClassConstraint);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
