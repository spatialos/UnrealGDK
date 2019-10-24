// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Utils/SpatialMetrics.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialMetrics() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialMetrics_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialMetrics();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
// End Cross Module References
	void USpatialMetrics::StaticRegisterNativesUSpatialMetrics()
	{
		UClass* Class = USpatialMetrics::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "SpatialModifySetting", &USpatialMetrics::execSpatialModifySetting },
			{ "SpatialStartRPCMetrics", &USpatialMetrics::execSpatialStartRPCMetrics },
			{ "SpatialStopRPCMetrics", &USpatialMetrics::execSpatialStopRPCMetrics },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics
	{
		struct SpatialMetrics_eventSpatialModifySetting_Parms
		{
			FString Name;
			float Value;
		};
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Value;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Name_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Name;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Value = { "Value", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialMetrics_eventSpatialModifySetting_Parms, Value), METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Name_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Name = { "Name", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialMetrics_eventSpatialModifySetting_Parms, Name), METADATA_PARAMS(Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Name_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Name_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Value,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::NewProp_Name,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SpatialMetrics.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialMetrics, nullptr, "SpatialModifySetting", sizeof(SpatialMetrics_eventSpatialModifySetting_Parms), Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020601, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SpatialMetrics.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialMetrics, nullptr, "SpatialStartRPCMetrics", 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020601, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics::Function_MetaDataParams[] = {
		{ "ModuleRelativePath", "Public/Utils/SpatialMetrics.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialMetrics, nullptr, "SpatialStopRPCMetrics", 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x00020601, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USpatialMetrics_NoRegister()
	{
		return USpatialMetrics::StaticClass();
	}
	struct Z_Construct_UClass_USpatialMetrics_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialMetrics_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USpatialMetrics_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USpatialMetrics_SpatialModifySetting, "SpatialModifySetting" }, // 3669804747
		{ &Z_Construct_UFunction_USpatialMetrics_SpatialStartRPCMetrics, "SpatialStartRPCMetrics" }, // 1671789365
		{ &Z_Construct_UFunction_USpatialMetrics_SpatialStopRPCMetrics, "SpatialStopRPCMetrics" }, // 1267879417
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialMetrics_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Utils/SpatialMetrics.h" },
		{ "ModuleRelativePath", "Public/Utils/SpatialMetrics.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialMetrics_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/Utils/SpatialMetrics.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialMetrics_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialMetrics, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialMetrics_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialMetrics_Statics::NewProp_NetDriver_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialMetrics_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialMetrics_Statics::NewProp_NetDriver,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialMetrics_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialMetrics>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialMetrics_Statics::ClassParams = {
		&USpatialMetrics::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_USpatialMetrics_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		ARRAY_COUNT(FuncInfo),
		ARRAY_COUNT(Z_Construct_UClass_USpatialMetrics_Statics::PropPointers),
		0,
		0x000000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialMetrics_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialMetrics_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialMetrics()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialMetrics_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialMetrics, 3450595141);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialMetrics>()
	{
		return USpatialMetrics::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialMetrics(Z_Construct_UClass_USpatialMetrics, &USpatialMetrics::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialMetrics"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialMetrics);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
