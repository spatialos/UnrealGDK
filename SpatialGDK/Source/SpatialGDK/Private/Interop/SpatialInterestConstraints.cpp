// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialInterestConstraints.h"

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Interest.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"
#include "UObject/UObjectIterator.h"

void UOrConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager, SpatialGDK::QueryConstraint& OutConstraint) const
{
	for (const UAbstractQueryConstraint* ConstraintData : Constraints)
	{
		if (ConstraintData != nullptr)
		{
			SpatialGDK::QueryConstraint NewConstraint;
			ConstraintData->CreateConstraint(ClassInfoManager, NewConstraint);
			if (NewConstraint.IsValid())
			{
				OutConstraint.OrConstraint.Add(NewConstraint);
			}
		}
	}
}

void UAndConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager, SpatialGDK::QueryConstraint& OutConstraint) const
{
	for (const UAbstractQueryConstraint* ConstraintData : Constraints)
	{
		if (ConstraintData != nullptr)
		{
			SpatialGDK::QueryConstraint NewConstraint;
			ConstraintData->CreateConstraint(ClassInfoManager, NewConstraint);
			if (NewConstraint.IsValid())
			{
				OutConstraint.AndConstraint.Add(NewConstraint);
			}
		}
	}
}

void USphereConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.SphereConstraint =
		SpatialGDK::SphereConstraint{ SpatialGDK::Coordinates::FromFVector(Center), static_cast<double>(Radius) / 100.0 };
}

void UCylinderConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
										   SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.CylinderConstraint =
		SpatialGDK::CylinderConstraint{ SpatialGDK::Coordinates::FromFVector(Center), static_cast<double>(Radius) / 100.0 };
}

void UBoxConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager, SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.BoxConstraint =
		SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center), SpatialGDK::Coordinates::FromFVector(EdgeLengths) };
}

void URelativeSphereConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
												 SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeSphereConstraint = SpatialGDK::RelativeSphereConstraint{ static_cast<double>(Radius) / 100.0 };
}

void URelativeCylinderConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
												   SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeCylinderConstraint = SpatialGDK::RelativeCylinderConstraint{ static_cast<double>(Radius) / 100.0 };
}

void URelativeBoxConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
											  SpatialGDK::QueryConstraint& OutConstraint) const
{
	OutConstraint.RelativeBoxConstraint = SpatialGDK::RelativeBoxConstraint{ SpatialGDK::Coordinates::FromFVector(EdgeLengths) };
}

void UCheckoutRadiusConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
												 SpatialGDK::QueryConstraint& OutConstraint) const
{
	if (!ActorClass.Get())
	{
		return;
	}

	SpatialGDK::QueryConstraint RadiusConstraint;
	RadiusConstraint.RelativeCylinderConstraint = SpatialGDK::RelativeCylinderConstraint{ static_cast<double>(Radius) / 100.0 };

	TArray<Worker_ComponentId> ComponentIds = ClassInfoManager.GetComponentIdsForClassHierarchy(*ActorClass.Get());
	SpatialGDK::QueryConstraint ActorClassConstraints;
	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		SpatialGDK::QueryConstraint ComponentTypeConstraint;
		ComponentTypeConstraint.ComponentConstraint = ComponentId;
		ActorClassConstraints.OrConstraint.Add(ComponentTypeConstraint);
	}

	if (RadiusConstraint.IsValid() && ActorClassConstraints.IsValid())
	{
		OutConstraint.AndConstraint.Add(RadiusConstraint);
		OutConstraint.AndConstraint.Add(ActorClassConstraints);
	}
}

void UActorClassConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
											 SpatialGDK::QueryConstraint& OutConstraint) const
{
	if (!ActorClass.Get())
	{
		return;
	}

	TArray<Worker_ComponentId> ComponentIds = ClassInfoManager.GetComponentIdsForClassHierarchy(*ActorClass.Get(), bIncludeDerivedClasses);
	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		SpatialGDK::QueryConstraint ComponentTypeConstraint;
		ComponentTypeConstraint.ComponentConstraint = ComponentId;
		OutConstraint.OrConstraint.Add(ComponentTypeConstraint);
	}
}

void UComponentClassConstraint::CreateConstraint(const USpatialClassInfoManager& ClassInfoManager,
												 SpatialGDK::QueryConstraint& OutConstraint) const
{
	if (!ComponentClass.Get())
	{
		return;
	}

	TArray<Worker_ComponentId> ComponentIds =
		ClassInfoManager.GetComponentIdsForClassHierarchy(*ComponentClass.Get(), bIncludeDerivedClasses);
	for (Worker_ComponentId ComponentId : ComponentIds)
	{
		SpatialGDK::QueryConstraint ComponentTypeConstraint;
		ComponentTypeConstraint.ComponentConstraint = ComponentId;
		OutConstraint.OrConstraint.Add(ComponentTypeConstraint);
	}
}
