// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetConnection.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialNetConnection() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetConnection_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetConnection();
	ONLINESUBSYSTEMUTILS_API UClass* Z_Construct_UClass_UIpConnection();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
// End Cross Module References
	void USpatialNetConnection::StaticRegisterNativesUSpatialNetConnection()
	{
	}
	UClass* Z_Construct_UClass_USpatialNetConnection_NoRegister()
	{
		return USpatialNetConnection::StaticClass();
	}
	struct Z_Construct_UClass_USpatialNetConnection_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WorkerAttribute_MetaData[];
#endif
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_WorkerAttribute;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_bReliableSpatialConnection_MetaData[];
#endif
		static void NewProp_bReliableSpatialConnection_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bReliableSpatialConnection;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialNetConnection_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UIpConnection,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetConnection_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialNetConnection.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetConnection.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_WorkerAttribute_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetConnection.h" },
	};
#endif
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_WorkerAttribute = { "WorkerAttribute", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialNetConnection, WorkerAttribute), METADATA_PARAMS(Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_WorkerAttribute_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_WorkerAttribute_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialNetConnection.h" },
	};
#endif
	void Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection_SetBit(void* Obj)
	{
		((USpatialNetConnection*)Obj)->bReliableSpatialConnection = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection = { "bReliableSpatialConnection", nullptr, (EPropertyFlags)0x0010000000000000, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USpatialNetConnection), &Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection_SetBit, METADATA_PARAMS(Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialNetConnection_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_WorkerAttribute,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialNetConnection_Statics::NewProp_bReliableSpatialConnection,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialNetConnection_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialNetConnection>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialNetConnection_Statics::ClassParams = {
		&USpatialNetConnection::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialNetConnection_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialNetConnection_Statics::PropPointers),
		0,
		0x001000ACu,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialNetConnection_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialNetConnection_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialNetConnection()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialNetConnection_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialNetConnection, 3476424014);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialNetConnection>()
	{
		return USpatialNetConnection::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialNetConnection(Z_Construct_UClass_USpatialNetConnection, &USpatialNetConnection::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialNetConnection"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialNetConnection);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
