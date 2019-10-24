// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialActorChannel.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialActorChannel() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialActorChannel_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialActorChannel();
	ENGINE_API UClass* Z_Construct_UClass_UActorChannel();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialReceiver_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialSender_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialNetDriver_NoRegister();
// End Cross Module References
	void USpatialActorChannel::StaticRegisterNativesUSpatialActorChannel()
	{
	}
	UClass* Z_Construct_UClass_USpatialActorChannel_NoRegister()
	{
		return USpatialActorChannel::StaticClass();
	}
	struct Z_Construct_UClass_USpatialActorChannel_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Receiver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Receiver;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Sender_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Sender;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_NetDriver_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_NetDriver;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialActorChannel_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UActorChannel,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialActorChannel_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialActorChannel.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialActorChannel.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Receiver_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialActorChannel.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Receiver = { "Receiver", nullptr, (EPropertyFlags)0x0040000000002000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialActorChannel, Receiver), Z_Construct_UClass_USpatialReceiver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Receiver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Receiver_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Sender_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialActorChannel.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Sender = { "Sender", nullptr, (EPropertyFlags)0x0040000000002000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialActorChannel, Sender), Z_Construct_UClass_USpatialSender_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Sender_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Sender_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_NetDriver_MetaData[] = {
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialActorChannel.h" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_NetDriver = { "NetDriver", nullptr, (EPropertyFlags)0x0040000000002000, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USpatialActorChannel, NetDriver), Z_Construct_UClass_USpatialNetDriver_NoRegister, METADATA_PARAMS(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_NetDriver_MetaData, ARRAY_COUNT(Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_NetDriver_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USpatialActorChannel_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Receiver,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_Sender,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USpatialActorChannel_Statics::NewProp_NetDriver,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialActorChannel_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialActorChannel>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialActorChannel_Statics::ClassParams = {
		&USpatialActorChannel::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USpatialActorChannel_Statics::PropPointers,
		nullptr,
		ARRAY_COUNT(DependentSingletons),
		0,
		ARRAY_COUNT(Z_Construct_UClass_USpatialActorChannel_Statics::PropPointers),
		0,
		0x001000A8u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialActorChannel_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialActorChannel_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialActorChannel()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialActorChannel_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialActorChannel, 196502099);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialActorChannel>()
	{
		return USpatialActorChannel::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialActorChannel(Z_Construct_UClass_USpatialActorChannel, &USpatialActorChannel::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialActorChannel"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialActorChannel);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
