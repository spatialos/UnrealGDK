// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/SimulatedPlayers/SimPlayerBPFunctionLibrary.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSimPlayerBPFunctionLibrary() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USimPlayerBPFunctionLibrary_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USimPlayerBPFunctionLibrary();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject_NoRegister();
// End Cross Module References
	void USimPlayerBPFunctionLibrary::StaticRegisterNativesUSimPlayerBPFunctionLibrary()
	{
		UClass* Class = USimPlayerBPFunctionLibrary::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "IsSimulatedPlayer", &USimPlayerBPFunctionLibrary::execIsSimulatedPlayer },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics
	{
		struct SimPlayerBPFunctionLibrary_eventIsSimulatedPlayer_Parms
		{
			const UObject* WorldContextObject;
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WorldContextObject_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_WorldContextObject;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SimPlayerBPFunctionLibrary_eventIsSimulatedPlayer_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SimPlayerBPFunctionLibrary_eventIsSimulatedPlayer_Parms), &Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_WorldContextObject_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_WorldContextObject = { "WorldContextObject", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SimPlayerBPFunctionLibrary_eventIsSimulatedPlayer_Parms, WorldContextObject), Z_Construct_UClass_UObject_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_WorldContextObject_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_WorldContextObject_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::NewProp_WorldContextObject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|SimulatedPlayer" },
		{ "ModuleRelativePath", "Public/SimulatedPlayers/SimPlayerBPFunctionLibrary.h" },
		{ "ToolTip", "Get whether this client is simulated.\nThis will return true for clients launched inside simulated player deployments,\nor simulated clients launched from the Editor." },
		{ "WorldContext", "WorldContextObject" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USimPlayerBPFunctionLibrary, nullptr, "IsSimulatedPlayer", sizeof(SimPlayerBPFunctionLibrary_eventIsSimulatedPlayer_Parms), Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USimPlayerBPFunctionLibrary_NoRegister()
	{
		return USimPlayerBPFunctionLibrary::StaticClass();
	}
	struct Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USimPlayerBPFunctionLibrary_IsSimulatedPlayer, "IsSimulatedPlayer" }, // 2328092837
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SimulatedPlayers/SimPlayerBPFunctionLibrary.h" },
		{ "ModuleRelativePath", "Public/SimulatedPlayers/SimPlayerBPFunctionLibrary.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USimPlayerBPFunctionLibrary>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::ClassParams = {
		&USimPlayerBPFunctionLibrary::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USimPlayerBPFunctionLibrary()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USimPlayerBPFunctionLibrary, 2576159996);
	template<> SPATIALGDK_API UClass* StaticClass<USimPlayerBPFunctionLibrary>()
	{
		return USimPlayerBPFunctionLibrary::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USimPlayerBPFunctionLibrary(Z_Construct_UClass_USimPlayerBPFunctionLibrary, &USimPlayerBPFunctionLibrary::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USimPlayerBPFunctionLibrary"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USimPlayerBPFunctionLibrary);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
