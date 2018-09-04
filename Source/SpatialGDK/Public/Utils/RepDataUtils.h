// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Net/RepLayout.h"

// Storage for a changelist created by the replication system when replicating from the server.
struct FPropertyChangeState
{
	const uint8* RESTRICT SourceData;
	const TArray<uint16> RepChanged; // changed replicated properties
	TArray<FRepLayoutCmd>& RepCmds;
	TArray<FHandleToCmdIndex>& RepBaseHandleToCmdIndex;
	TArray<FRepParentCmd>& Parents;
	const TArray<uint16> HandoverChanged; // changed handover properties
};

// A structure containing information about a replicated property.
class FRepHandleData
{
public:
	FRepHandleData(UClass* Class, TArray<FName> PropertyNames, TArray<int32> PropertyIndices, ELifetimeCondition InCondition, ELifetimeRepNotifyCondition InRepNotifyCondition) :
		Condition(InCondition),
		RepNotifyCondition(InRepNotifyCondition),
		Offset(0)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		UStruct* CurrentContainerType = Class;
		for (FName PropertyName : PropertyNames)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the end) is not a container."));
			UProperty* CurProperty = CurrentContainerType->FindPropertyByName(PropertyName);
			PropertyChain.Add(CurProperty);
			if (UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty))
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				CurrentContainerType = nullptr;
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];

		// Calculate offset by summing the offsets of each property in the chain.
		for (int i = 0; i < PropertyChain.Num(); i++)
		{
			const UProperty* CurProperty = PropertyChain[i];
			// Calculate the static array offset of this specific property, using its index and its parents indices.
			int32 IndexOffset = PropertyIndices[i] * CurProperty->ElementSize;
			Offset += CurProperty->GetOffset_ForInternal();
			Offset += IndexOffset;
		}
	}

	FORCEINLINE uint8* GetPropertyData(uint8* Container) const
	{
		return Container + Offset;
	}


	FORCEINLINE const uint8* GetPropertyData(const uint8* Container) const
	{
		return Container + Offset;
	}

	TArray<UProperty*> PropertyChain;
	UProperty* Property;
	ELifetimeCondition Condition;
	ELifetimeRepNotifyCondition RepNotifyCondition;

private:
	int32 Offset;
};

// A structure containing information about a handover property.
class FHandoverHandleData
{
public:
	FHandoverHandleData(UClass* Class, TArray<FName> PropertyNames, TArray<int32> InPropertyIndices) :
		SubobjectProperty(false),
		Offset(0),
		PropertyIndices(InPropertyIndices)
	{
		// Build property chain.
		check(PropertyNames.Num() > 0);
		check(PropertyNames.Num() == PropertyIndices.Num());
		UStruct* CurrentContainerType = Class;
		for (int i = 0; i < PropertyNames.Num(); ++i)
		{
			checkf(CurrentContainerType, TEXT("A property in the chain (except the end) is not a container."));
			const FName& PropertyName = PropertyNames[i];
			UProperty* CurProperty = CurrentContainerType->FindPropertyByName(PropertyName);
			check(CurProperty);
			PropertyChain.Add(CurProperty);
			if (!SubobjectProperty)
			{
				Offset += CurProperty->GetOffset_ForInternal();
				Offset += PropertyIndices[i] * CurProperty->ElementSize;
			}

			if (UStructProperty* StructProperty = Cast<UStructProperty>(CurProperty))
			{
				CurrentContainerType = StructProperty->Struct;
			}
			else
			{
				if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(CurProperty))
				{
					CurrentContainerType = ObjectProperty->PropertyClass;
					SubobjectProperty = true; // We are now recursing into a subobjects properties.
					Offset = 0;
				}
				else
				{
					// We should only encounter a non-container style property if this is the final property in the chain.
					// Otherwise, the above check will be hit.
					CurrentContainerType = nullptr;
				}
			}
		}
		Property = PropertyChain[PropertyChain.Num() - 1];
	}

	FORCEINLINE uint8* GetPropertyData(uint8* Container) const
	{
		if (SubobjectProperty)
		{
			uint8* Data = Container;
			for (int i = 0; i < PropertyChain.Num(); ++i)
			{
				Data += PropertyChain[i]->GetOffset_ForInternal();
				Data += PropertyIndices[i] * PropertyChain[i]->ElementSize;

				// If we're not the last property in the chain.
				if (i < (PropertyChain.Num() - 1))
				{
					// Handover property chains can cross into subobjects, so we will need to deal with objects which are not inlined into the container.
					if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(PropertyChain[i]))
					{
						UObject* PropertyValue = ObjectProperty->GetObjectPropertyValue(Data);
						Data = (uint8*)PropertyValue;
					}
				}
			}
			return Data;
		}
		else
		{
			return Container + Offset;
		}
	}

	FORCEINLINE const uint8* GetPropertyData(const uint8* Container) const
	{
		return GetPropertyData((uint8*)Container);
	}

	TArray<UProperty*> PropertyChain;
	UProperty* Property;
	TArray<int32> PropertyIndices;

private:
	bool SubobjectProperty;  // If this is true, then this property refers to a property within a subobject.
	uint32 Offset;
};

// A map from rep handle to rep handle data.
using FRepHandlePropertyMap = TMap<uint16, FRepHandleData>;

// A map from handover handle to handover handle data.
using FHandoverHandlePropertyMap = TMap<uint16, FHandoverHandleData>;
