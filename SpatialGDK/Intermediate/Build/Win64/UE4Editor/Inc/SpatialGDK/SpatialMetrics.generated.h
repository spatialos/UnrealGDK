// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SpatialMetrics_generated_h
#error "SpatialMetrics.generated.h already included, missing '#pragma once' in SpatialMetrics.h"
#endif
#define SPATIALGDK_SpatialMetrics_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execSpatialModifySetting) \
	{ \
		P_GET_PROPERTY(UStrProperty,Z_Param_Name); \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Value); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialModifySetting(Z_Param_Name,Z_Param_Value); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execSpatialStopRPCMetrics) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialStopRPCMetrics(); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execSpatialStartRPCMetrics) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialStartRPCMetrics(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execSpatialModifySetting) \
	{ \
		P_GET_PROPERTY(UStrProperty,Z_Param_Name); \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Value); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialModifySetting(Z_Param_Name,Z_Param_Value); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execSpatialStopRPCMetrics) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialStopRPCMetrics(); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execSpatialStartRPCMetrics) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->SpatialStartRPCMetrics(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialMetrics(); \
	friend struct Z_Construct_UClass_USpatialMetrics_Statics; \
public: \
	DECLARE_CLASS(USpatialMetrics, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialMetrics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialMetrics(); \
	friend struct Z_Construct_UClass_USpatialMetrics_Statics; \
public: \
	DECLARE_CLASS(USpatialMetrics, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialMetrics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialMetrics(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialMetrics) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialMetrics); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialMetrics); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialMetrics(USpatialMetrics&&); \
	NO_API USpatialMetrics(const USpatialMetrics&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialMetrics(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialMetrics(USpatialMetrics&&); \
	NO_API USpatialMetrics(const USpatialMetrics&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialMetrics); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialMetrics); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialMetrics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__NetDriver() { return STRUCT_OFFSET(USpatialMetrics, NetDriver); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_20_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h_23_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialMetrics>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetrics_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
