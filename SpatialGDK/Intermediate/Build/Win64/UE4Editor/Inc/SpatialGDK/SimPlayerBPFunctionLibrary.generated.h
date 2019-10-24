// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class UObject;
#ifdef SPATIALGDK_SimPlayerBPFunctionLibrary_generated_h
#error "SimPlayerBPFunctionLibrary.generated.h already included, missing '#pragma once' in SimPlayerBPFunctionLibrary.h"
#endif
#define SPATIALGDK_SimPlayerBPFunctionLibrary_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execIsSimulatedPlayer) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USimPlayerBPFunctionLibrary::IsSimulatedPlayer(Z_Param_WorldContextObject); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execIsSimulatedPlayer) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USimPlayerBPFunctionLibrary::IsSimulatedPlayer(Z_Param_WorldContextObject); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSimPlayerBPFunctionLibrary(); \
	friend struct Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics; \
public: \
	DECLARE_CLASS(USimPlayerBPFunctionLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USimPlayerBPFunctionLibrary)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_INCLASS \
private: \
	static void StaticRegisterNativesUSimPlayerBPFunctionLibrary(); \
	friend struct Z_Construct_UClass_USimPlayerBPFunctionLibrary_Statics; \
public: \
	DECLARE_CLASS(USimPlayerBPFunctionLibrary, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USimPlayerBPFunctionLibrary)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USimPlayerBPFunctionLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USimPlayerBPFunctionLibrary) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USimPlayerBPFunctionLibrary); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USimPlayerBPFunctionLibrary); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USimPlayerBPFunctionLibrary(USimPlayerBPFunctionLibrary&&); \
	NO_API USimPlayerBPFunctionLibrary(const USimPlayerBPFunctionLibrary&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USimPlayerBPFunctionLibrary(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USimPlayerBPFunctionLibrary(USimPlayerBPFunctionLibrary&&); \
	NO_API USimPlayerBPFunctionLibrary(const USimPlayerBPFunctionLibrary&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USimPlayerBPFunctionLibrary); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USimPlayerBPFunctionLibrary); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USimPlayerBPFunctionLibrary)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_PRIVATE_PROPERTY_OFFSET
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_9_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h_12_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USimPlayerBPFunctionLibrary>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_SimulatedPlayers_SimPlayerBPFunctionLibrary_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
