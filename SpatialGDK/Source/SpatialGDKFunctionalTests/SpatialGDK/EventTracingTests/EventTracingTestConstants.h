// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "EventTracingTestConstants.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UEventTracingTestConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static FName GetReceiveOpEventName() { return ReceiveOpEventName; }
	static FName GetSendPropertyUpdatesEventName() { return SendPropertyUpdatesEventName; }
	static FName GetReceivePropertyUpdateEventName() { return ReceivePropertyUpdateEventName; }
	static FName GetSendRPCEventName() { return SendRPCEventName; }
	static FName GetProcessRPCEventName() { return ProcessRPCEventName; }
	static FName GetComponentUpdateEventName() { return ComponentUpdateEventName; }
	static FName GetMergeComponentUpdateEventName() { return MergeComponentUpdateEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserProcessRPCEventName() { return UserProcessRPCEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserReceivePropertyEventName() { return UserReceivePropertyEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserReceiveComponentPropertyEventName() { return UserReceiveComponentPropertyEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserSendPropertyEventName() { return UserSendPropertyEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserSendComponentPropertyEventName() { return UserSendComponentPropertyEventName; }

	UFUNCTION(BlueprintCallable, Category = "EventTracingTest")
		static FName GetUserSendRPCEventName() { return UserSendRPCEventName; }

private:
	const static FName ReceiveOpEventName;
	const static FName SendPropertyUpdatesEventName;
	const static FName ReceivePropertyUpdateEventName;
	const static FName SendRPCEventName;
	const static FName ProcessRPCEventName;
	const static FName ComponentUpdateEventName;
	const static FName MergeComponentUpdateEventName;
	const static FName UserProcessRPCEventName;
	const static FName UserReceivePropertyEventName;
	const static FName UserReceiveComponentPropertyEventName;
	const static FName UserSendPropertyEventName;
	const static FName UserSendComponentPropertyEventName;
	const static FName UserSendRPCEventName;
};
