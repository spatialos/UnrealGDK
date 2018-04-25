// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include "TestSchema.h"

#include "TestType1.h"

#include "CoreMinimal.h"
#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "TestCommandUserTypeCommandResponder.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class SPATIALOS_API UTestCommandUserTypeCommandResponder : public UObject
{
    GENERATED_BODY()

  public:
    UTestCommandUserTypeCommandResponder();
	UTestCommandUserTypeCommandResponder* Init(
        const TWeakPtr<SpatialOSConnection>& InConnection,
        worker::RequestId<
            worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandUserType>
		> InRequestId,
        UTestType1* InRequest, 
		const std::string& InCallerWorkerId
	);

    UFUNCTION(BlueprintPure, Category = "TestCommandResponseTypesComponent")
    UTestType1* GetRequest();

	UFUNCTION(BlueprintPure, Category = "TestCommandResponseTypesComponent")
    FString GetCallerWorkerId();

    UFUNCTION(BlueprintCallable, Category = "TestCommandResponseTypesComponent")
    void SendResponse(UTestType1* response);

  private:

    TWeakPtr<SpatialOSConnection> Connection;

    worker::RequestId<worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandUserType>>
        RequestId;

	UPROPERTY()
    UTestType1* Request;

	FString CallerWorkerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTestCommandUserTypeCommand, UTestCommandUserTypeCommandResponder*, responder);
