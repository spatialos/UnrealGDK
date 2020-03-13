// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "TransientUObjectEditor.generated.h"

// Utility class to create Editor tools exposing a UObject Field and automatically adding Exec UFUNCTION as buttons.
UCLASS(Blueprintable, Abstract)
class SPATIALGDKEDITOR_API UTransientUObjectEditor : public UObject
{
	GENERATED_BODY()
public:

	template <typename T>
	static void LaunchTransientUObjectEditor(const FString& EditorName)
	{
		LaunchTransientUObjectEditor(EditorName, T::StaticClass());
	}

private:
	static void LaunchTransientUObjectEditor(const FString& EditorName, UClass* ObjectClass);
};
