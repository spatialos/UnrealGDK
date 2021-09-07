#include "EventTracerComponent.h"

#include "Interop/Connection/SpatialEventTracerUserInterface.h"
#include "Interop/Connection/UserSpanId.h"
#include "Net/UnrealNetwork.h"

UEventTracerComponent::UEventTracerComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UEventTracerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEventTracerComponent, TestInt);
}

void UEventTracerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bUseEventTracing && OwnerHasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UEventTracerComponent::TimerFunction, 1.0f, true);
	}
}

void UEventTracerComponent::TimerFunction()
{
	// Create a user trace event for sending a property update
	FUserSpanId SpanId = USpatialEventTracerUserInterface::TraceEvent(this, "user.send_component_property", "");
	USpatialEventTracerUserInterface::TraceProperty(this, this, SpanId);

	TestInt++;

	// Create a user trace event for sending an RPC
	FEventTracerAddDataDelegate AddDataDelegate;
	AddDataDelegate.BindLambda([](SpatialGDK::FSpatialTraceEventDataBuilder& Builder) {
		Builder.AddKeyValue("Message", "Delegate value");
	});

	SpanId = USpatialEventTracerUserInterface::TraceEvent(this, "user.send_rpc", "", AddDataDelegate);

	FEventTracerRPCDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UEventTracerComponent, RunOnClient));
	USpatialEventTracerUserInterface::TraceRPC(this, Delegate, SpanId);
}

void UEventTracerComponent::OnRepTestInt()
{
	if (!OwnerHasAuthority())
	{
		FUserSpanId CauseSpanId;
		if (USpatialEventTracerUserInterface::GetActiveSpanId(this, CauseSpanId))
		{
			USpatialEventTracerUserInterface::TraceEventWithCauses(this, "user.receive_component_property", "", { CauseSpanId });
		}
	}
}

void UEventTracerComponent::RunOnClient_Implementation()
{
	// Create a user trace event for receiving an RPC

	FUserSpanId CauseSpanId;
	if (USpatialEventTracerUserInterface::GetActiveSpanId(this, CauseSpanId))
	{
		USpatialEventTracerUserInterface::TraceEventWithCauses(this, "user.process_rpc", "", { CauseSpanId });
	}
}

bool UEventTracerComponent::OwnerHasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr && Owner->HasAuthority();
}
