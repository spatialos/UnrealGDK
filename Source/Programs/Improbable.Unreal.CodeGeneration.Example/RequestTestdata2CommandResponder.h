// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "TestSchema.h"

#include "TestType2.h"

#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "RequestTestdata2CommandResponder.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API URequestTestdata2CommandResponder : public UObject
{
    GENERATED_BODY()

  public:
    URequestTestdata2CommandResponder();
	URequestTestdata2CommandResponder* Init(
        const TWeakPtr<SpatialOSConnection>& InConnection,
        worker::RequestId<
            worker::IncomingCommandRequest<test::TestData1::Commands::RequestTestdata2>
		> InRequestId,
        UTestType1* InRequest, 
		const std::string& InCallerWorkerId
	);

    UFUNCTION(BlueprintPure, Category = "TestData1Component")
    UTestType1* GetRequest();

	UFUNCTION(BlueprintPure, Category = "TestData1Component")
    FString GetCallerWorkerId();

    UFUNCTION(BlueprintCallable, Category = "TestData1Component")
    void SendResponse(UTestType2* response);

  private:

    TWeakPtr<SpatialOSConnection> Connection;

    worker::RequestId<worker::IncomingCommandRequest<test::TestData1::Commands::RequestTestdata2>>
        RequestId;

	UPROPERTY()
    UTestType1* Request;

	FString CallerWorkerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRequestTestdata2Command, URequestTestdata2CommandResponder*, responder);
