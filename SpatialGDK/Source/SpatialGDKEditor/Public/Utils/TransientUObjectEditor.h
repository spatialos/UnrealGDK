// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "TransientUObjectEditor.generated.h"

DECLARE_DELEGATE(FOnTransientUObjectEditorClosed)

class SWindow;

// Utility class to create Editor tools exposing a UObject Field and automatically adding Exec UFUNCTION as buttons.
UCLASS(Blueprintable, Abstract)
class SPATIALGDKEDITOR_API UTransientUObjectEditor : public UObject
{
	GENERATED_BODY()
public:
	template <typename T>
	static T* LaunchTransientUObjectEditor(const FText& EditorName, TSharedPtr<SWindow> ParentWindow)
	{
		return Cast<T>(LaunchTransientUObjectEditor(EditorName, T::StaticClass(), ParentWindow));
	}

private:
	static UTransientUObjectEditor* LaunchTransientUObjectEditor(const FText& EditorName, UClass* ObjectClass,
																 TSharedPtr<SWindow> ParentWindow);
};
