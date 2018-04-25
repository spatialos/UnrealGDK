// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestSchema.h"


#include "TestEmptyComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestEmptyComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UTestEmptyComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestEmptyComponentUpdate* Init(const test::TestEmpty::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestEmptyComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::TestEmpty::Update GetUnderlying();

  private:
    friend class UTestEmptyComponent;
	friend class UCallbackDispatcher;

	UTestEmptyComponentUpdate* InitInternal(const test::TestEmpty::Update& underlying);
	UTestEmptyComponentUpdate* ResetInternal();

    const test::TestEmpty::Update GetUnderlyingInternal();


    test::TestEmpty::Update Underlying;
	static test::TestEmpty::Update DefaultUnderlying;
};
