// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_GlobalStateManager_generated_h
#error "GlobalStateManager.generated.h already included, missing '#pragma once' in GlobalStateManager.h"
#endif
#define SPATIALGDK_GlobalStateManager_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_RPC_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_RPC_WRAPPERS_NO_PURE_DECLS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUGlobalStateManager(); \
	friend struct Z_Construct_UClass_UGlobalStateManager_Statics; \
public: \
	DECLARE_CLASS(UGlobalStateManager, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(UGlobalStateManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_INCLASS \
private: \
	static void StaticRegisterNativesUGlobalStateManager(); \
	friend struct Z_Construct_UClass_UGlobalStateManager_Statics; \
public: \
	DECLARE_CLASS(UGlobalStateManager, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(UGlobalStateManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UGlobalStateManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UGlobalStateManager) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UGlobalStateManager); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UGlobalStateManager); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UGlobalStateManager(UGlobalStateManager&&); \
	NO_API UGlobalStateManager(const UGlobalStateManager&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UGlobalStateManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UGlobalStateManager(UGlobalStateManager&&); \
	NO_API UGlobalStateManager(const UGlobalStateManager&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UGlobalStateManager); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UGlobalStateManager); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UGlobalStateManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__NetDriver() { return STRUCT_OFFSET(UGlobalStateManager, NetDriver); } \
	FORCEINLINE static uint32 __PPO__StaticComponentView() { return STRUCT_OFFSET(UGlobalStateManager, StaticComponentView); } \
	FORCEINLINE static uint32 __PPO__Sender() { return STRUCT_OFFSET(UGlobalStateManager, Sender); } \
	FORCEINLINE static uint32 __PPO__Receiver() { return STRUCT_OFFSET(UGlobalStateManager, Receiver); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_24_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h_27_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class UGlobalStateManager>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_GlobalStateManager_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
