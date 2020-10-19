// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "UObject/UnrealType.h"

#if ENGINE_MINOR_VERSION <= 24
#define GDK_PROPERTY(Type) U##Type
#define GDK_CASTFIELD Cast
#else
#define GDK_PROPERTY(Type) F##Type
#define GDK_CASTFIELD CastField
#endif
