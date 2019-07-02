// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialInterestConstraints.h"

#include "Schema/Interest.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"
#include "Utils/SchemaDatabase.h"
#include "UObject/UObjectIterator.h"

namespace
{
void AddTypeHierarchyToConstraint(const UClass* BaseType, const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint, const bool bIncludeDerivedTypes = true)
{
	if (bIncludeDerivedTypes)
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			const UClass* Class = *It;
			check(Class);
			if (Class->IsChildOf(BaseType))
			{
				const uint32 ComponentId = SchemaDatabase.GetComponentIdForClass(*Class);
				if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
				{
					SpatialGDK::QueryConstraint ClassComponentConstraint;
					ClassComponentConstraint.ComponentConstraint = ComponentId;
					OutConstraint.OrConstraint.Add(ClassComponentConstraint);
				}
			}
		}
	}
	else
	{
		const uint32 ComponentId = SchemaDatabase.GetComponentIdForClass(*BaseType);
		if (ComponentId != SpatialConstants::INVALID_COMPONENT_ID)
		{
			OutConstraint.ComponentConstraint = ComponentId;
		}
	}
}
}

void UOrConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	for (const UAbstractQueryConstraint* ConstraintData : Constraints)
	{
		if (ConstraintData)
		{
			SpatialGDK::QueryConstraint NewConstraint;
			ConstraintData->CreateConstraint(SchemaDatabase, NewConstraint);
			if (NewConstraint.IsValid())
			{
				OutConstraint.OrConstraint.Add(NewConstraint);
			}
		}
	}
}

void UAndConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	for (const UAbstractQueryConstraint* ConstraintData : Constraints)
	{
		if (ConstraintData)
		{
			SpatialGDK::QueryConstraint NewConstraint; ConstraintData->CreateConstraint(SchemaDatabase, NewConstraint);
			if (NewConstraint.IsValid())
			{
				OutConstraint.AndConstraint.Add(NewConstraint);
			}
		}
	}
}

void USphereConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.SphereConstraint = SpatialGDK::SphereConstraint{ SpatialGDK::Coordinates::FromFVector(Center), Radius };
}

void UCylinderConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.CylinderConstraint = SpatialGDK::CylinderConstraint{ SpatialGDK::Coordinates::FromFVector(Center), Radius };
}

void UBoxConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center), SpatialGDK::Coordinates::FromFVector(EdgeLengths) };
}

void URelativeSphereConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeSphereConstraint = SpatialGDK::RelativeSphereConstraint{ Radius };
}

void URelativeCylinderConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeCylinderConstraint = SpatialGDK::RelativeCylinderConstraint{ Radius };
}

void URelativeBoxConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeBoxConstraint = SpatialGDK::RelativeBoxConstraint{ SpatialGDK::Coordinates::FromFVector(EdgeLengths) };
}

void UCheckoutRadiusConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	const double RadiusMeters = static_cast<double>(Radius) / 100.0;
	SpatialGDK::QueryConstraint RadiusConstraint;
	RadiusConstraint.RelativeCylinderConstraint = SpatialGDK::RelativeCylinderConstraint{ RadiusMeters };

	SpatialGDK::QueryConstraint ActorClassConstraints;
	constexpr bool bIncludeDerivedTypes = true;
	AddTypeHierarchyToConstraint(ActorClass, SchemaDatabase, ActorClassConstraints, bIncludeDerivedTypes);

	if (RadiusConstraint.IsValid() && ActorClassConstraints.IsValid())
	{
		OutConstraint.AndConstraint.Add(RadiusConstraint);
		OutConstraint.AndConstraint.Add(ActorClassConstraints);
	}
}

void UActorClassConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	AddTypeHierarchyToConstraint(ActorClass, SchemaDatabase, OutConstraint, bIncludeDerivedClasses);
}

void UComponentClassConstraint::CreateConstraint(const USchemaDatabase& SchemaDatabase, SpatialGDK::QueryConstraint& OutConstraint) const
{
	AddTypeHierarchyToConstraint(ComponentClass, SchemaDatabase, OutConstraint, bIncludeDerivedClasses);
}
