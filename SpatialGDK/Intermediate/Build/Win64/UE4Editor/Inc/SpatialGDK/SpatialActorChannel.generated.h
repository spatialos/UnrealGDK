// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SpatialActorChannel_generated_h
#error "SpatialActorChannel.generated.h already included, missing '#pragma once' in SpatialActorChannel.h"
#endif
#define SPATIALGDK_SpatialActorChannel_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_RPC_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_RPC_WRAPPERS_NO_PURE_DECLS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialActorChannel(); \
	friend struct Z_Construct_UClass_USpatialActorChannel_Statics; \
public: \
	DECLARE_CLASS(USpatialActorChannel, UActorChannel, COMPILED_IN_FLAGS(0 | CLASS_Transient), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialActorChannel)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialActorChannel(); \
	friend struct Z_Construct_UClass_USpatialActorChannel_Statics; \
public: \
	DECLARE_CLASS(USpatialActorChannel, UActorChannel, COMPILED_IN_FLAGS(0 | CLASS_Transient), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialActorChannel)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialActorChannel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialActorChannel) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialActorChannel); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialActorChannel); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialActorChannel(USpatialActorChannel&&); \
	NO_API USpatialActorChannel(const USpatialActorChannel&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialActorChannel(USpatialActorChannel&&); \
	NO_API USpatialActorChannel(const USpatialActorChannel&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialActorChannel); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialActorChannel); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialActorChannel)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__NetDriver() { return STRUCT_OFFSET(USpatialActorChannel, NetDriver); } \
	FORCEINLINE static uint32 __PPO__Sender() { return STRUCT_OFFSET(USpatialActorChannel, Sender); } \
	FORCEINLINE static uint32 __PPO__Receiver() { return STRUCT_OFFSET(USpatialActorChannel, Receiver); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_22_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h_25_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialActorChannel>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialActorChannel_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
