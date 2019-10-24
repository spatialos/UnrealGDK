// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SnapshotManager_generated_h
#error "SnapshotManager.generated.h already included, missing '#pragma once' in SnapshotManager.h"
#endif
#define SPATIALGDK_SnapshotManager_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_RPC_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_RPC_WRAPPERS_NO_PURE_DECLS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSnapshotManager(); \
	friend struct Z_Construct_UClass_USnapshotManager_Statics; \
public: \
	DECLARE_CLASS(USnapshotManager, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USnapshotManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_INCLASS \
private: \
	static void StaticRegisterNativesUSnapshotManager(); \
	friend struct Z_Construct_UClass_USnapshotManager_Statics; \
public: \
	DECLARE_CLASS(USnapshotManager, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USnapshotManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USnapshotManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USnapshotManager) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USnapshotManager); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USnapshotManager); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USnapshotManager(USnapshotManager&&); \
	NO_API USnapshotManager(const USnapshotManager&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USnapshotManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USnapshotManager(USnapshotManager&&); \
	NO_API USnapshotManager(const USnapshotManager&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USnapshotManager); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USnapshotManager); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USnapshotManager)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__NetDriver() { return STRUCT_OFFSET(USnapshotManager, NetDriver); } \
	FORCEINLINE static uint32 __PPO__GlobalStateManager() { return STRUCT_OFFSET(USnapshotManager, GlobalStateManager); } \
	FORCEINLINE static uint32 __PPO__Receiver() { return STRUCT_OFFSET(USnapshotManager, Receiver); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_21_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h_24_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USnapshotManager>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_Interop_SnapshotManager_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
