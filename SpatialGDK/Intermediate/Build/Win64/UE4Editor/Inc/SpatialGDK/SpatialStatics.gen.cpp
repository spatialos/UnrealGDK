// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Utils/SpatialStatics.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialStatics() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStatics_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialStatics();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor();
	ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass();
	COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject_NoRegister();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass();
	SPATIALGDK_API UFunction* Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled();
// End Cross Module References
	void USpatialStatics::StaticRegisterNativesUSpatialStatics()
	{
		UClass* Class = USpatialStatics::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "GetActorGroupForActor", &USpatialStatics::execGetActorGroupForActor },
			{ "GetActorGroupForClass", &USpatialStatics::execGetActorGroupForClass },
			{ "IsActorGroupOwner", &USpatialStatics::execIsActorGroupOwner },
			{ "IsActorGroupOwnerForActor", &USpatialStatics::execIsActorGroupOwnerForActor },
			{ "IsActorGroupOwnerForClass", &USpatialStatics::execIsActorGroupOwnerForClass },
			{ "IsSpatialNetworkingEnabled", &USpatialStatics::execIsSpatialNetworkingEnabled },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics
	{
		struct SpatialStatics_eventGetActorGroupForActor_Parms
		{
			const AActor* Actor;
			FName ReturnValue;
		};
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Actor_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Actor;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventGetActorGroupForActor_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_Actor_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_Actor = { "Actor", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventGetActorGroupForActor_Parms, Actor), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_Actor_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_Actor_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::NewProp_Actor,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|Offloading" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns the ActorGroup this Actor belongs to." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "GetActorGroupForActor", sizeof(SpatialStatics_eventGetActorGroupForActor_Parms), Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics
	{
		struct SpatialStatics_eventGetActorGroupForClass_Parms
		{
			const UObject* WorldContextObject;
			const TSubclassOf<AActor>  ActorClass;
			FName ReturnValue;
		};
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FClassPropertyParams NewProp_ActorClass;
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
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventGetActorGroupForClass_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ActorClass_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FClassPropertyParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ActorClass = { "ActorClass", nullptr, (EPropertyFlags)0x0014000000000082, UE4CodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventGetActorGroupForClass_Parms, ActorClass), Z_Construct_UClass_AActor_NoRegister, Z_Construct_UClass_UClass, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ActorClass_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ActorClass_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_WorldContextObject_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_WorldContextObject = { "WorldContextObject", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventGetActorGroupForClass_Parms, WorldContextObject), Z_Construct_UClass_UObject_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_WorldContextObject_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_WorldContextObject_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_ActorClass,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::NewProp_WorldContextObject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|Offloading" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns the ActorGroup this Actor Class belongs to." },
		{ "WorldContext", "WorldContextObject" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "GetActorGroupForClass", sizeof(SpatialStatics_eventGetActorGroupForClass_Parms), Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics
	{
		struct SpatialStatics_eventIsActorGroupOwner_Parms
		{
			const UObject* WorldContextObject;
			FName ActorGroup;
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorGroup_MetaData[];
#endif
		static const UE4CodeGen_Private::FNamePropertyParams NewProp_ActorGroup;
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
	void Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SpatialStatics_eventIsActorGroupOwner_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SpatialStatics_eventIsActorGroupOwner_Parms), &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ActorGroup_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FNamePropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ActorGroup = { "ActorGroup", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Name, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventIsActorGroupOwner_Parms, ActorGroup), METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ActorGroup_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ActorGroup_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_WorldContextObject_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_WorldContextObject = { "WorldContextObject", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventIsActorGroupOwner_Parms, WorldContextObject), Z_Construct_UClass_UObject_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_WorldContextObject_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_WorldContextObject_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_ActorGroup,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::NewProp_WorldContextObject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|Offloading" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns true if the current Worker Type owns this Actor Group.\nEquivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled." },
		{ "WorldContext", "WorldContextObject" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "IsActorGroupOwner", sizeof(SpatialStatics_eventIsActorGroupOwner_Parms), Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics
	{
		struct SpatialStatics_eventIsActorGroupOwnerForActor_Parms
		{
			const AActor* Actor;
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Actor_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Actor;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SpatialStatics_eventIsActorGroupOwnerForActor_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SpatialStatics_eventIsActorGroupOwnerForActor_Parms), &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_Actor_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_Actor = { "Actor", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventIsActorGroupOwnerForActor_Parms, Actor), Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_Actor_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_Actor_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::NewProp_Actor,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|Offloading" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns true if the current Worker Type owns the Actor Group this Actor belongs to.\nEquivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "IsActorGroupOwnerForActor", sizeof(SpatialStatics_eventIsActorGroupOwnerForActor_Parms), Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics
	{
		struct SpatialStatics_eventIsActorGroupOwnerForClass_Parms
		{
			const UObject* WorldContextObject;
			const TSubclassOf<AActor>  ActorClass;
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ActorClass_MetaData[];
#endif
		static const UE4CodeGen_Private::FClassPropertyParams NewProp_ActorClass;
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
	void Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SpatialStatics_eventIsActorGroupOwnerForClass_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SpatialStatics_eventIsActorGroupOwnerForClass_Parms), &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ActorClass_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FClassPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ActorClass = { "ActorClass", nullptr, (EPropertyFlags)0x0014000000000082, UE4CodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventIsActorGroupOwnerForClass_Parms, ActorClass), Z_Construct_UClass_AActor_NoRegister, Z_Construct_UClass_UClass, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ActorClass_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ActorClass_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_WorldContextObject_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_WorldContextObject = { "WorldContextObject", nullptr, (EPropertyFlags)0x0010000000000082, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SpatialStatics_eventIsActorGroupOwnerForClass_Parms, WorldContextObject), Z_Construct_UClass_UObject_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_WorldContextObject_MetaData, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_WorldContextObject_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ReturnValue,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_ActorClass,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::NewProp_WorldContextObject,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS|Offloading" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns true if the current Worker Type owns the Actor Group this Actor Class belongs to.\nEquivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled." },
		{ "WorldContext", "WorldContextObject" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "IsActorGroupOwnerForClass", sizeof(SpatialStatics_eventIsActorGroupOwnerForClass_Parms), Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics
	{
		struct SpatialStatics_eventIsSpatialNetworkingEnabled_Parms
		{
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SpatialStatics_eventIsSpatialNetworkingEnabled_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SpatialStatics_eventIsSpatialNetworkingEnabled_Parms), &Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::Function_MetaDataParams[] = {
		{ "Category", "SpatialOS" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
		{ "ToolTip", "Returns true if SpatialOS Networking is enabled." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USpatialStatics, nullptr, "IsSpatialNetworkingEnabled", sizeof(SpatialStatics_eventIsSpatialNetworkingEnabled_Parms), Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::PropPointers, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::Function_MetaDataParams, ARRAY_COUNT(Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USpatialStatics_NoRegister()
	{
		return USpatialStatics::StaticClass();
	}
	struct Z_Construct_UClass_USpatialStatics_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialStatics_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USpatialStatics_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USpatialStatics_GetActorGroupForActor, "GetActorGroupForActor" }, // 324156972
		{ &Z_Construct_UFunction_USpatialStatics_GetActorGroupForClass, "GetActorGroupForClass" }, // 15788178
		{ &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwner, "IsActorGroupOwner" }, // 1011139053
		{ &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForActor, "IsActorGroupOwnerForActor" }, // 1337502359
		{ &Z_Construct_UFunction_USpatialStatics_IsActorGroupOwnerForClass, "IsActorGroupOwnerForClass" }, // 2753502843
		{ &Z_Construct_UFunction_USpatialStatics_IsSpatialNetworkingEnabled, "IsSpatialNetworkingEnabled" }, // 1818460066
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialStatics_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Utils/SpatialStatics.h" },
		{ "ModuleRelativePath", "Public/Utils/SpatialStatics.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialStatics_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialStatics>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialStatics_Statics::ClassParams = {
		&USpatialStatics::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_USpatialStatics_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialStatics_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialStatics()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialStatics_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialStatics, 1898899402);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialStatics>()
	{
		return USpatialStatics::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialStatics(Z_Construct_UClass_USpatialStatics, &USpatialStatics::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialStatics"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialStatics);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
