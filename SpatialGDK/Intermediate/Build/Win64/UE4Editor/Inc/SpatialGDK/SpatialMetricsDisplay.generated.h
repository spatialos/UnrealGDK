// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
struct FWorkerStats;
#ifdef SPATIALGDK_SpatialMetricsDisplay_generated_h
#error "SpatialMetricsDisplay.generated.h already included, missing '#pragma once' in SpatialMetricsDisplay.h"
#endif
#define SPATIALGDK_SpatialMetricsDisplay_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_14_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FWorkerStats_Statics; \
	SPATIALGDK_API static class UScriptStruct* StaticStruct();


template<> SPATIALGDK_API UScriptStruct* StaticStruct<struct FWorkerStats>();

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_RPC_WRAPPERS \
	virtual bool ServerUpdateWorkerStats_Validate(const float , FWorkerStats const& ); \
	virtual void ServerUpdateWorkerStats_Implementation(const float Time, FWorkerStats const& OneWorkerStats); \
 \
	DECLARE_FUNCTION(execServerUpdateWorkerStats) \
	{ \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Time); \
		P_GET_STRUCT(FWorkerStats,Z_Param_OneWorkerStats); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		if (!P_THIS->ServerUpdateWorkerStats_Validate(Z_Param_Time,Z_Param_OneWorkerStats)) \
		{ \
			RPC_ValidateFailed(TEXT("ServerUpdateWorkerStats_Validate")); \
			return; \
		} \
		P_THIS->ServerUpdateWorkerStats_Implementation(Z_Param_Time,Z_Param_OneWorkerStats); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execToggleStatDisplay) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->ToggleStatDisplay(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_RPC_WRAPPERS_NO_PURE_DECLS \
	virtual bool ServerUpdateWorkerStats_Validate(const float , FWorkerStats const& ); \
	virtual void ServerUpdateWorkerStats_Implementation(const float Time, FWorkerStats const& OneWorkerStats); \
 \
	DECLARE_FUNCTION(execServerUpdateWorkerStats) \
	{ \
		P_GET_PROPERTY(UFloatProperty,Z_Param_Time); \
		P_GET_STRUCT(FWorkerStats,Z_Param_OneWorkerStats); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		if (!P_THIS->ServerUpdateWorkerStats_Validate(Z_Param_Time,Z_Param_OneWorkerStats)) \
		{ \
			RPC_ValidateFailed(TEXT("ServerUpdateWorkerStats_Validate")); \
			return; \
		} \
		P_THIS->ServerUpdateWorkerStats_Implementation(Z_Param_Time,Z_Param_OneWorkerStats); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execToggleStatDisplay) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		P_THIS->ToggleStatDisplay(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_EVENT_PARMS \
	struct SpatialMetricsDisplay_eventServerUpdateWorkerStats_Parms \
	{ \
		float Time; \
		FWorkerStats OneWorkerStats; \
	};


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_CALLBACK_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesASpatialMetricsDisplay(); \
	friend struct Z_Construct_UClass_ASpatialMetricsDisplay_Statics; \
public: \
	DECLARE_CLASS(ASpatialMetricsDisplay, AInfo, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(ASpatialMetricsDisplay) \
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_INCLASS \
private: \
	static void StaticRegisterNativesASpatialMetricsDisplay(); \
	friend struct Z_Construct_UClass_ASpatialMetricsDisplay_Statics; \
public: \
	DECLARE_CLASS(ASpatialMetricsDisplay, AInfo, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(ASpatialMetricsDisplay) \
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API ASpatialMetricsDisplay(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ASpatialMetricsDisplay) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ASpatialMetricsDisplay); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ASpatialMetricsDisplay); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API ASpatialMetricsDisplay(ASpatialMetricsDisplay&&); \
	NO_API ASpatialMetricsDisplay(const ASpatialMetricsDisplay&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API ASpatialMetricsDisplay(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API ASpatialMetricsDisplay(ASpatialMetricsDisplay&&); \
	NO_API ASpatialMetricsDisplay(const ASpatialMetricsDisplay&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, ASpatialMetricsDisplay); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(ASpatialMetricsDisplay); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(ASpatialMetricsDisplay)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__WorkerStats() { return STRUCT_OFFSET(ASpatialMetricsDisplay, WorkerStats); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_33_PROLOG \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_EVENT_PARMS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_CALLBACK_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_CALLBACK_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h_37_ENHANCED_CONSTRUCTORS \
static_assert(false, "Unknown access specifier for GENERATED_BODY() macro in class SpatialMetricsDisplay."); \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class ASpatialMetricsDisplay>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialMetricsDisplay_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
