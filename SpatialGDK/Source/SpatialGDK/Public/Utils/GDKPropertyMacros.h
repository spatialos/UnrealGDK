// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "UObject/UnrealType.h"
#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MINOR_VERSION <= 24
	#define GDK_PROPERTY(Type) U##Type
	#define GDK_CASTFIELD Cast
#else
	#define GDK_PROPERTY(Type) F##Type
	#define GDK_CASTFIELD CastField
#endif
