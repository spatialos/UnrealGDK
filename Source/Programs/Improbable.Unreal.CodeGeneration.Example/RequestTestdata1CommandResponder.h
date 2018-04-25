// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "TestSchema.h"

#include "TestType1.h"

#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "RequestTestdata1CommandResponder.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API URequestTestdata1CommandResponder : public UObject
{
    GENERATED_BODY()

  public:
    URequestTestdata1CommandResponder();
	URequestTestdata1CommandResponder* Init(
        const TWeakPtr<SpatialOSConnection>& InConnection,
        worker::RequestId<
            worker::IncomingCommandRequest<test::TestData2::Commands::RequestTestdata1>
		> InRequestId,
        UTestType2* InRequest, 
		const std::string& InCallerWorkerId
	);

    UFUNCTION(BlueprintPure, Category = "TestData2Component")
    UTestType2* GetRequest();

	UFUNCTION(BlueprintPure, Category = "TestData2Component")
    FString GetCallerWorkerId();

    UFUNCTION(BlueprintCallable, Category = "TestData2Component")
    void SendResponse(UTestType1* response);

  private:

    TWeakPtr<SpatialOSConnection> Connection;

    worker::RequestId<worker::IncomingCommandRequest<test::TestData2::Commands::RequestTestdata1>>
        RequestId;

	UPROPERTY()
    UTestType2* Request;

	FString CallerWorkerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRequestTestdata1Command, URequestTestdata1CommandResponder*, responder);
