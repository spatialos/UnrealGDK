// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SpatialGDK/Public/EngineClasses/SpatialPendingNetGame.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSpatialPendingNetGame() {}
// Cross Module References
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPendingNetGame_NoRegister();
	SPATIALGDK_API UClass* Z_Construct_UClass_USpatialPendingNetGame();
	ENGINE_API UClass* Z_Construct_UClass_UPendingNetGame();
	UPackage* Z_Construct_UPackage__Script_SpatialGDK();
// End Cross Module References
	void USpatialPendingNetGame::StaticRegisterNativesUSpatialPendingNetGame()
	{
	}
	UClass* Z_Construct_UClass_USpatialPendingNetGame_NoRegister()
	{
		return USpatialPendingNetGame::StaticClass();
	}
	struct Z_Construct_UClass_USpatialPendingNetGame_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USpatialPendingNetGame_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UPendingNetGame,
		(UObject* (*)())Z_Construct_UPackage__Script_SpatialGDK,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USpatialPendingNetGame_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "EngineClasses/SpatialPendingNetGame.h" },
		{ "ModuleRelativePath", "Public/EngineClasses/SpatialPendingNetGame.h" },
		{ "ToolTip", "UPendingNetGame needs to have its dllexport defined: \"class ENGINE_API UPendingNetGame\". This can count as a bug, we can submit a PR." },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USpatialPendingNetGame_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USpatialPendingNetGame>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USpatialPendingNetGame_Statics::ClassParams = {
		&USpatialPendingNetGame::StaticClass,
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
		0x000000A8u,
		0x00000000u,
		METADATA_PARAMS(Z_Construct_UClass_USpatialPendingNetGame_Statics::Class_MetaDataParams, ARRAY_COUNT(Z_Construct_UClass_USpatialPendingNetGame_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USpatialPendingNetGame()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USpatialPendingNetGame_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USpatialPendingNetGame, 3905956365);
	template<> SPATIALGDK_API UClass* StaticClass<USpatialPendingNetGame>()
	{
		return USpatialPendingNetGame::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USpatialPendingNetGame(Z_Construct_UClass_USpatialPendingNetGame, &USpatialPendingNetGame::StaticClass, TEXT("/Script/SpatialGDK"), TEXT("USpatialPendingNetGame"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USpatialPendingNetGame);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
