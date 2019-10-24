// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SpatialWorkerFlags_generated_h
#error "SpatialWorkerFlags.generated.h already included, missing '#pragma once' in SpatialWorkerFlags.h"
#endif
#define SPATIALGDK_SpatialWorkerFlags_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_9_DELEGATE \
struct _Script_SpatialGDK_eventOnWorkerFlagsUpdated_Parms \
{ \
	FString FlagName; \
	FString FlagValue; \
}; \
static inline void FOnWorkerFlagsUpdated_DelegateWrapper(const FMulticastScriptDelegate& OnWorkerFlagsUpdated, const FString& FlagName, const FString& FlagValue) \
{ \
	_Script_SpatialGDK_eventOnWorkerFlagsUpdated_Parms Parms; \
	Parms.FlagName=FlagName; \
	Parms.FlagValue=FlagValue; \
	OnWorkerFlagsUpdated.ProcessMulticastDelegate<UObject>(&Parms); \
}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_8_DELEGATE \
struct _Script_SpatialGDK_eventOnWorkerFlagsUpdatedBP_Parms \
{ \
	FString FlagName; \
	FString FlagValue; \
}; \
static inline void FOnWorkerFlagsUpdatedBP_DelegateWrapper(const FScriptDelegate& OnWorkerFlagsUpdatedBP, const FString& FlagName, const FString& FlagValue) \
{ \
	_Script_SpatialGDK_eventOnWorkerFlagsUpdatedBP_Parms Parms; \
	Parms.FlagName=FlagName; \
	Parms.FlagValue=FlagValue; \
	OnWorkerFlagsUpdatedBP.ProcessDelegate<UObject>(&Parms); \
}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execUnbindFromOnWorkerFlagsUpdated) \
	{ \
		P_GET_PROPERTY_REF(UDelegateProperty,Z_Param_Out_InDelegate); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		USpatialWorkerFlags::UnbindFromOnWorkerFlagsUpdated(FOnWorkerFlagsUpdatedBP(Z_Param_Out_InDelegate)); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execBindToOnWorkerFlagsUpdated) \
	{ \
		P_GET_PROPERTY_REF(UDelegateProperty,Z_Param_Out_InDelegate); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		USpatialWorkerFlags::BindToOnWorkerFlagsUpdated(FOnWorkerFlagsUpdatedBP(Z_Param_Out_InDelegate)); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetWorkerFlag) \
	{ \
		P_GET_PROPERTY(UStrProperty,Z_Param_Name); \
		P_GET_PROPERTY_REF(UStrProperty,Z_Param_Out_OutValue); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialWorkerFlags::GetWorkerFlag(Z_Param_Name,Z_Param_Out_OutValue); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execUnbindFromOnWorkerFlagsUpdated) \
	{ \
		P_GET_PROPERTY_REF(UDelegateProperty,Z_Param_Out_InDelegate); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		USpatialWorkerFlags::UnbindFromOnWorkerFlagsUpdated(FOnWorkerFlagsUpdatedBP(Z_Param_Out_InDelegate)); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execBindToOnWorkerFlagsUpdated) \
	{ \
		P_GET_PROPERTY_REF(UDelegateProperty,Z_Param_Out_InDelegate); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		USpatialWorkerFlags::BindToOnWorkerFlagsUpdated(FOnWorkerFlagsUpdatedBP(Z_Param_Out_InDelegate)); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetWorkerFlag) \
	{ \
		P_GET_PROPERTY(UStrProperty,Z_Param_Name); \
		P_GET_PROPERTY_REF(UStrProperty,Z_Param_Out_OutValue); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialWorkerFlags::GetWorkerFlag(Z_Param_Name,Z_Param_Out_OutValue); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialWorkerFlags(); \
	friend struct Z_Construct_UClass_USpatialWorkerFlags_Statics; \
public: \
	DECLARE_CLASS(USpatialWorkerFlags, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialWorkerFlags)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialWorkerFlags(); \
	friend struct Z_Construct_UClass_USpatialWorkerFlags_Statics; \
public: \
	DECLARE_CLASS(USpatialWorkerFlags, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialWorkerFlags)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialWorkerFlags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialWorkerFlags) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialWorkerFlags); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialWorkerFlags); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialWorkerFlags(USpatialWorkerFlags&&); \
	NO_API USpatialWorkerFlags(const USpatialWorkerFlags&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialWorkerFlags(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialWorkerFlags(USpatialWorkerFlags&&); \
	NO_API USpatialWorkerFlags(const USpatialWorkerFlags&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialWorkerFlags); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialWorkerFlags); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialWorkerFlags)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_PRIVATE_PROPERTY_OFFSET
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_11_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h_14_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialWorkerFlags>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialWorkerFlags_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
