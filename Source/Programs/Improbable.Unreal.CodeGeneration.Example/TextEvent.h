// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestSchema.h"

#include <string>
#include "TextEvent.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTextEvent : public UObject
{
    GENERATED_BODY()

  public:
    UTextEvent();
    UTextEvent* Init(const test::TextEvent& underlying);

    UFUNCTION(BlueprintPure, Category = "TextEvent")
    FString GetText();
	UFUNCTION(BlueprintCallable, Category = "TextEvent")
    UTextEvent* SetText(FString text);


	test::TextEvent GetUnderlying();

  private:
    TUniquePtr<test::TextEvent> Underlying;
};

 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextEventDelegate, UTextEvent*, newEvent);
