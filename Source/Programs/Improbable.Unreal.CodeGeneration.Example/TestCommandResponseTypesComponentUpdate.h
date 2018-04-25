// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.h"
#include "TestSchema.h"


#include "TestCommandResponseTypesComponentUpdate.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestCommandResponseTypesComponentUpdate : public UComponentUpdateOpWrapperBase
{
    GENERATED_BODY()

  public:
    UTestCommandResponseTypesComponentUpdate();
    
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestCommandResponseTypesComponentUpdate* Init(const test::TestCommandResponseTypes::Update& underlying);
	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
	UTestCommandResponseTypesComponentUpdate* Reset();

	DEPRECATED(12.1, "This class is deprecated, please access the Component properties directly and use automatic component updates or explicitly trigger a manual update with TriggerManualComponentUpdate.")
    const test::TestCommandResponseTypes::Update GetUnderlying();

  private:
    friend class UTestCommandResponseTypesComponent;
	friend class UCallbackDispatcher;

	UTestCommandResponseTypesComponentUpdate* InitInternal(const test::TestCommandResponseTypes::Update& underlying);
	UTestCommandResponseTypesComponentUpdate* ResetInternal();

    const test::TestCommandResponseTypes::Update GetUnderlyingInternal();


    test::TestCommandResponseTypes::Update Underlying;
	static test::TestCommandResponseTypes::Update DefaultUnderlying;
};
