// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceEventBuilder.h"

#include "Utils/SpatialActorUtils.h"

namespace SpatialGDK
{
FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(const char* InType)
	: SpatialTraceEvent(InType, "")
{
}

FSpatialTraceEventBuilder::FSpatialTraceEventBuilder(const char* InType, const FString& InMessage)
	: SpatialTraceEvent(InType, InMessage)
{
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddObject(const UObject* Object)
{
	if (Object != nullptr)
	{
		FString Key = TEXT("Object");
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			Key = TEXT("Actor");
			AddKeyValue(TEXT("ActorPosition"), Actor->GetTransform().GetTranslation().ToString());
			AddKeyValue(TEXT("ActorRoot"), GetNameSafe(SpatialGDK::GetHierarchyRoot(Actor)));
		}
		AddKeyValue(MoveTemp(Key), Object->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddObject(const FString& Key, const UObject* Object)
{
	if (Object != nullptr)
	{
		if (const AActor* Actor = Cast<AActor>(Object))
		{
			AddKeyValue(TEXT("ActorPosition"), Actor->GetTransform().GetTranslation().ToString());
		}
		AddKeyValue(Key, Object->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFunction(const UFunction* Function)
{
	if (Function != nullptr)
	{
		AddKeyValue(TEXT("Function"), Function->GetName());
	}
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddEntityId(const Worker_EntityId EntityId)
{
	AddKeyValue(TEXT("EntityId"), FString::FromInt(EntityId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddComponentId(const Worker_ComponentId ComponentId)
{
	AddKeyValue(TEXT("ComponentId"), FString::FromInt(ComponentId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddFieldId(const uint32 FieldId)
{
	AddKeyValue(TEXT("FieldId"), FString::FromInt(FieldId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddNewWorkerId(const uint32 NewWorkerId)
{
	AddKeyValue(TEXT("NewWorkerId"), FString::FromInt(NewWorkerId));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddCommand(const FString& Command)
{
	AddKeyValue(TEXT("Command"), Command);
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddRequestID(const int64 RequestID)
{
	AddKeyValue(TEXT("RequestID"), FString::FromInt(RequestID));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddNetRole(const ENetRole Role)
{
	AddKeyValue(TEXT("NetRole"), NetRoleToString(Role));
	return *this;
}

FSpatialTraceEventBuilder FSpatialTraceEventBuilder::AddKeyValue(FString Key, FString Value)
{
	SpatialTraceEvent.AddData(MoveTemp(Key), MoveTemp(Value));
	return *this;
}

FSpatialTraceEvent FSpatialTraceEventBuilder::Get() &&
{
	return MoveTemp(SpatialTraceEvent);
}
} // namespace SpatialGDK
