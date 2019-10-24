// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialGameInstance.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialGameInstance() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialGameInstance_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialGameInstance();
	ENGINE_API UClass* Z_Construct_UClass_UGameInstance();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialWorkerConnection_NoRegister();
// End Cross Module References
	void USpatialGameInstance::StaticRegisterNativesUSpatialGameInstance()
	{
	}
	UClass* Z_Construct_UClass_USpatialGameInstance_NoRegister()
	{
		return USpatialGameInstance::StaticClass();
	}
	struct Z_Construct_UClass_USpatialGameInstance_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bPreventAutoConnectWithLocator_MetaData[];
#endif
		static void NewProp_bPreventAutoConnectWithLocator_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bPreventAutoConnectWithLocator;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialConnection_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_SpatialConnection;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialGameInstance_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UGameInstance,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGameInstance_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialGameInstance.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialGameInstance.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialGameInstance.h" },
		{ "ToolTip", "If this flag is set to true standalone clients will not attempt to connect to a deployment automatically if a 'loginToken' exists in arguments." },
	};
#endif
	void Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator_SetBit(void* Obj)
	{
		((USpatialGameInstance*)Obj)->bPreventAutoConnectWithLocator = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator = { "bPreventAutoConnectWithLocator", nullptr, (EPropertyFlags)0x0040000000004000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialGameInstance), &Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_SpatialConnection_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialGameInstance.h" },
		{ "ToolTip", "SpatialConnection is stored here for persistence between map travels." },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_SpatialConnection = { "SpatialConnection", nullptr, (EPropertyFlags)0x0040000000000000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialGameInstance, SpatialConnection), Z_Construct_UClass_USpatialWorkerConnection_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_SpatialConnection_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_SpatialConnection_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialGameInstance_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_bPreventAutoConnectWithLocator,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialGameInstance_Statics::NewProp_SpatialConnection,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialGameInstance_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialGameInstance>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialGameInstance_Statics::ClassParams = {
		&USpatialGameInstance::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialGameInstance_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialGameInstance_Statics::PropPointers),
		0,
		0x001000ACu,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialGameInstance_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialGameInstance_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialGameInstance()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialGameInstance_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialGameInstance, 1651606576);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialGameInstance>()
	{
		return USpatialGameInstance::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialGameInstance(Z_Construct_UClass_USpatialGameInstance, &USpatialGameInstance::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialGameInstance"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialGameInstance);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
