// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SPATIALGDK_SpatialGameInstance_generated_h
#error "SpatialGameInstance.generated.h already included, missing '#pragma once' in SpatialGameInstance.h"
#endif
#define SPATIALGDK_SpatialGameInstance_generated_h

#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_RPC_WRAPPERS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_RPC_WRAPPERS_NO_PURE_DECLS
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSpatialGameInstance(); \
	friend struct Z_Construct_UClass_USpatialGameInstance_Statics; \
public: \
	DECLARE_CLASS(USpatialGameInstance, UGameInstance, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialGameInstance) \
	static const TCHAR* StaticConfigName() {return TEXT("Engine");} \



#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_INCLASS \
private: \
	static void StaticRegisterNativesUSpatialGameInstance(); \
	friend struct Z_Construct_UClass_USpatialGameInstance_Statics; \
public: \
	DECLARE_CLASS(USpatialGameInstance, UGameInstance, COMPILED_IN_FLAGS(0 | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/SpatialGDK"), NO_API) \
	DECLARE_SERIALIZER(USpatialGameInstance) \
	static const TCHAR* StaticConfigName() {return TEXT("Engine");} \



#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialGameInstance) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialGameInstance); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialGameInstance); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialGameInstance(USpatialGameInstance&&); \
	NO_API USpatialGameInstance(const USpatialGameInstance&); \
public:


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USpatialGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USpatialGameInstance(USpatialGameInstance&&); \
	NO_API USpatialGameInstance(const USpatialGameInstance&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USpatialGameInstance); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USpatialGameInstance); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USpatialGameInstance)


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__SpatialConnection() { return STRUCT_OFFSET(USpatialGameInstance, SpatialConnection); } \
	FORCEINLINE static uint32 __PPO__bPreventAutoConnectWithLocator() { return STRUCT_OFFSET(USpatialGameInstance, bPreventAutoConnectWithLocator); }


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_17_PROLOG
#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_RPC_WRAPPERS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_INCLASS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_PRIVATE_PROPERTY_OFFSET \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_RPC_WRAPPERS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_INCLASS_NO_PURE_DECLS \
	Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h_20_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SPATIALGDK_API UClass* StaticClass<class USpatialGameInstance>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Game_Plugins_UnrealGDK_SpatialGDK_Source_SpatialGDK_Public_EngineClasses_SpatialGameInstance_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
