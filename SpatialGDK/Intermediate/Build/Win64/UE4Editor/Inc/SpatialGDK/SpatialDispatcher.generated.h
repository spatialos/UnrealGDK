// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SpatialDispatcher_generated_h
#error "SpatialDispatcher.generated.h already included, missing '#pragma once' in SpatialDispatcher.h"
#endif
#define SPATIALGDK_SpatialDispatcher_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_RPC_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_RPC_WRAPPERS_NO_PURE_DECLS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialDispatcher(); \
	friend struct Z_Construct_UClass_USpatialDispatcher_Statics; \
public: \
	DECLARE_CLASS(USpatialDispatcher, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialDispatcher)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialDispatcher(); \
	friend struct Z_Construct_UClass_USpatialDispatcher_Statics; \
public: \
	DECLARE_CLASS(USpatialDispatcher, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialDispatcher)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialDispatcher(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialDispatcher) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialDispatcher); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialDispatcher); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialDispatcher(USpatialDispatcher&&); \
	NO_API USpatialDispatcher(const USpatialDispatcher&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialDispatcher(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialDispatcher(USpatialDispatcher&&); \
	NO_API USpatialDispatcher(const USpatialDispatcher&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialDispatcher); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialDispatcher); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialDispatcher)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__NetDriver() { return STRUCT_OFFSET(USpatialDispatcher, NetDriver); } \
	FORCEINLINE static uint32 __PPO__Receiver() { return STRUCT_OFFSET(USpatialDispatcher, Receiver); } \
	FORCEINLINE static uint32 __PPO__StaticComponentView() { return STRUCT_OFFSET(USpatialDispatcher, StaticComponentView); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_24_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h_27_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialDispatcher>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SpatialDispatcher_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
