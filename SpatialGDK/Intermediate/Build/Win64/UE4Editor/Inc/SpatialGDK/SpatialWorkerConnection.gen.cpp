// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/Interop/Connection/SpatialWorkerConnection.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialWorkerConnection() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialWorkerConnection_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialWorkerConnection();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
// End Cross Module References
	void USpatialWorkerConnection::StaticRegisterNativesUSpatialWorkerConnection()
	{
	}
	UClass* Z_Construct_UClass_USpatialWorkerConnection_NoRegister()
	{
		return USpatialWorkerConnection::StaticClass();
	}
	struct Z_Construct_UClass_USpatialWorkerConnection_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialWorkerConnection_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialWorkerConnection_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Interop/Connection/SpatialWorkerConnection.h" },
		{ "ModuleRelativePath", "Public/Interop/Connection/SpatialWorkerConnection.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialWorkerConnection_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialWorkerConnection>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialWorkerConnection_Statics::ClassParams = {
		&USpatialWorkerConnection::StaticClass,
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
		METADATA_PARAMS(Z_Construct_UClass_USpatialWorkerConnection_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialWorkerConnection_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialWorkerConnection()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialWorkerConnection_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialWorkerConnection, 1683213653);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialWorkerConnection>()
	{
		return USpatialWorkerConnection::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialWorkerConnection(Z_Construct_UClass_USpatialWorkerConnection, &USpatialWorkerConnection::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialWorkerConnection"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialWorkerConnection);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
