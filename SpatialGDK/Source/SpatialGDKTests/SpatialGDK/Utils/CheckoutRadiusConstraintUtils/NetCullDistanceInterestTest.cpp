// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "HAL/IPlatformFileProfilerWrapper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/ScopeTryLock.h"

#include "Utils/Interest/NetCullDistanceInterest.h"

#define CHECKOUT_RADIUS_CONSTRAINT_TEST(TestName) GDK_TEST(Core, NetCullDistanceInterest, TestName)

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKCheckoutRadiusTest, Log, All);
DEFINE_LOG_CATEGORY(LogSpatialGDKCheckoutRadiusTest);

// Run tests inside the SpatialGDK namespace in order to test static functions.
namespace SpatialGDK
{
CHECKOUT_RADIUS_CONSTRAINT_TEST(GIVEN_actor_type_to_radius_map_WHEN_radius_is_duplicated_THEN_correctly_dedupes)
{
	float Radius = 5.f;
	TMap<UClass*, float> Map;
	UClass* Class1 = NewObject<UClass>();
	UClass* Class2 = NewObject<UClass>();
	Map.Add(Class1, Radius);
	Map.Add(Class2, Radius);

	TMap<float, TArray<UClass*>> DedupedMap = NetCullDistanceInterest::DedupeDistancesAcrossActorTypes(Map);

	int32 ExpectedSize = 1;
	TestTrue("There is only one entry in the map", DedupedMap.Num() == ExpectedSize);

	TArray<UClass*> Classes = DedupedMap[Radius];
	TArray<UClass*> ExpectedClasses;
	ExpectedClasses.Add(Class1);
	ExpectedClasses.Add(Class2);

	TestTrue("All UClasses are accounted for", Classes == ExpectedClasses);

	return true;
}

CHECKOUT_RADIUS_CONSTRAINT_TEST(GIVEN_actor_type_to_radius_map_WHEN_radius_is_not_duplicated_THEN_does_not_dedupe)
{
	float Radius1 = 5.f;
	float Radius2 = 6.f;
	TMap<UClass*, float> Map;
	UClass* Class1 = NewObject<UClass>();
	UClass* Class2 = NewObject<UClass>();
	Map.Add(Class1, Radius1);
	Map.Add(Class2, Radius2);

	TMap<float, TArray<UClass*>> DedupedMap = NetCullDistanceInterest::DedupeDistancesAcrossActorTypes(Map);

	int32 ExpectedSize = 2;
	TestTrue("There are two entries in the map", DedupedMap.Num() == ExpectedSize);

	TArray<UClass*> Classes = DedupedMap[Radius1];
	TArray<UClass*> ExpectedClasses;
	ExpectedClasses.Add(Class1);

	TestTrue("Class for first radius is present", Classes == ExpectedClasses);

	Classes = DedupedMap[Radius2];
	ExpectedClasses.Empty();
	ExpectedClasses.Add(Class2);

	TestTrue("Class for second radius is present", Classes == ExpectedClasses);

	return true;
}
} // namespace SpatialGDK
