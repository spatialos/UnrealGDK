// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class ULevel;
class UWorld;
#ifdef SPATIALGDK_SpatialNetDriver_generated_h
#error "SpatialNetDriver.generated.h already included, missing '#pragma once' in SpatialNetDriver.h"
#endif
#define SPATIALGDK_SpatialNetDriver_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execOnLevelAddedToWorld) \
	{ \
		P_GET_OBJECT(ULevel,Z_Param_LoadedLevel); \
		P_GET_OBJECT(UWorld,Z_Param_OwningWorld); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->OnLevelAddedToWorld(Z_Param_LoadedLevel,Z_Param_OwningWorld); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execOnMapLoaded) \
	{ \
		P_GET_OBJECT(UWorld,Z_Param_LoadedWorld); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->OnMapLoaded(Z_Param_LoadedWorld); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execOnLevelAddedToWorld) \
	{ \
		P_GET_OBJECT(ULevel,Z_Param_LoadedLevel); \
		P_GET_OBJECT(UWorld,Z_Param_OwningWorld); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->OnLevelAddedToWorld(Z_Param_LoadedLevel,Z_Param_OwningWorld); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execOnMapLoaded) \
	{ \
		P_GET_OBJECT(UWorld,Z_Param_LoadedWorld); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->OnMapLoaded(Z_Param_LoadedWorld); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialNetDriver(); \
	friend struct Z_Construct_UClass_USpatialNetDriver_Statics; \
public: \
	DECLARE_CLASS(USpatialNetDriver, UIpNetDriver, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialNetDriver)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialNetDriver(); \
	friend struct Z_Construct_UClass_USpatialNetDriver_Statics; \
public: \
	DECLARE_CLASS(USpatialNetDriver, UIpNetDriver, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialNetDriver)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialNetDriver(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialNetDriver) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialNetDriver); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialNetDriver); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialNetDriver(USpatialNetDriver&&); \
	NO_API USpatialNetDriver(const USpatialNetDriver&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialNetDriver(USpatialNetDriver&&); \
	NO_API USpatialNetDriver(const USpatialNetDriver&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialNetDriver); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialNetDriver); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialNetDriver)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_PRIVATE_PROPERTY_OFFSET
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_46_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h_49_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialNetDriver>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialNetDriver_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
