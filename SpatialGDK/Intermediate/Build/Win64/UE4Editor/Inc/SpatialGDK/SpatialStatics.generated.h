// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class UObject;
class AActor;
#ifdef SPATIALGDK_SpatialStatics_generated_h
#error "SpatialStatics.generated.h already included, missing '#pragma once' in SpatialStatics.h"
#endif
#define SPATIALGDK_SpatialStatics_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execGetActorGroupForClass) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_OBJECT(UClass,Z_Param_ActorClass); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FName*)Z_Param__Result=USpatialStatics::GetActorGroupForClass(Z_Param_WorldContextObject,Z_Param_ActorClass); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetActorGroupForActor) \
	{ \
		P_GET_OBJECT(AActor,Z_Param_Actor); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FName*)Z_Param__Result=USpatialStatics::GetActorGroupForActor(Z_Param_Actor); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwner) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_PROPERTY(UNameProperty,Z_Param_ActorGroup); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwner(Z_Param_WorldContextObject,Z_Param_ActorGroup); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwnerForClass) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_OBJECT(UClass,Z_Param_ActorClass); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwnerForClass(Z_Param_WorldContextObject,Z_Param_ActorClass); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwnerForActor) \
	{ \
		P_GET_OBJECT(AActor,Z_Param_Actor); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwnerForActor(Z_Param_Actor); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsSpatialNetworkingEnabled) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsSpatialNetworkingEnabled(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execGetActorGroupForClass) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_OBJECT(UClass,Z_Param_ActorClass); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FName*)Z_Param__Result=USpatialStatics::GetActorGroupForClass(Z_Param_WorldContextObject,Z_Param_ActorClass); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetActorGroupForActor) \
	{ \
		P_GET_OBJECT(AActor,Z_Param_Actor); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FName*)Z_Param__Result=USpatialStatics::GetActorGroupForActor(Z_Param_Actor); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwner) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_PROPERTY(UNameProperty,Z_Param_ActorGroup); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwner(Z_Param_WorldContextObject,Z_Param_ActorGroup); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwnerForClass) \
	{ \
		P_GET_OBJECT(UObject,Z_Param_WorldContextObject); \
		P_GET_OBJECT(UClass,Z_Param_ActorClass); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwnerForClass(Z_Param_WorldContextObject,Z_Param_ActorClass); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsActorGroupOwnerForActor) \
	{ \
		P_GET_OBJECT(AActor,Z_Param_Actor); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsActorGroupOwnerForActor(Z_Param_Actor); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execIsSpatialNetworkingEnabled) \
	{ \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=USpatialStatics::IsSpatialNetworkingEnabled(); \
		P_NATIVE_END; \
	}


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialStatics(); \
	friend struct Z_Construct_UClass_USpatialStatics_Statics; \
public: \
	DECLARE_CLASS(USpatialStatics, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialStatics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialStatics(); \
	friend struct Z_Construct_UClass_USpatialStatics_Statics; \
public: \
	DECLARE_CLASS(USpatialStatics, UBlueprintFunctionLibrary, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialStatics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialStatics(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialStatics) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialStatics); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialStatics); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialStatics(USpatialStatics&&); \
	NO_API USpatialStatics(const USpatialStatics&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialStatics(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialStatics(USpatialStatics&&); \
	NO_API USpatialStatics(const USpatialStatics&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialStatics); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialStatics); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialStatics)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_PRIVATE_PROPERTY_OFFSET
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_11_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h_14_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialStatics>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Utils_SpatialStatics_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
