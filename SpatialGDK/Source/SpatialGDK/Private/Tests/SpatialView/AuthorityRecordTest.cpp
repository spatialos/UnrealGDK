// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "SpatialView/AuthorityRecord.h"

#define AUTHORITYRECORD_TEST(TestName) GDK_TEST(Core, AuthorityRecord, TestName)

using namespace SpatialGDK;

namespace
{
class AuthorityChangeRecordFixture
{
public:
	const Worker_EntityId kTestEntityId = 1337;
	const Worker_ComponentId kTestComponentId = 1338;

	const EntityComponentId kEntityComponentId{ kTestEntityId, kTestComponentId };

	AuthorityRecord Record;

	TArray<EntityComponentId> ExpectedAuthorityGained;
	TArray<EntityComponentId> ExpectedAuthorityLost;
	TArray<EntityComponentId> ExpectedAuthorityLostTemporarily;
};
} // anonymous namespace

AUTHORITYRECORD_TEST(GIVEN_EmptyAuthorityRecord_WHEN_set_to_authoritative_THEN_AuthorityRecord_has_AuthorityGainedRecord)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;

	// WHEN
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);

	// THEN
	Fixture.ExpectedAuthorityGained.Push(Fixture.kEntityComponentId);
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}

AUTHORITYRECORD_TEST(GIVEN_AuthorityRecord_with_AuthoritativeRecord_WHEN_set_to_NonAuthoritative_THEN_AuthorityRecord_has_no_records)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);

	// WHEN
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	// THEN
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}

AUTHORITYRECORD_TEST(GIVEN_empty_AuthorityRecord_WHEN_set_to_NonAuthoritative_THEN_has_AuthorityLostRecord)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;

	// WHEN
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	// THEN
	Fixture.ExpectedAuthorityLost.Push(Fixture.kEntityComponentId);
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}

AUTHORITYRECORD_TEST(GIVEN_AuthorityRecord_with_NonAuthoritativeRecord_WHEN_set_to_Authoritative_THEN_has_AuthorityLostTemporarilyRecord)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	// WHEN
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);

	// THEN
	Fixture.ExpectedAuthorityLostTemporarily.Push(Fixture.kEntityComponentId);
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}

AUTHORITYRECORD_TEST(
	GIVEN_AuthorityRecord_with_NonAuthoritativeRecord_WHEN_set_to_Authoritative_and_NonAuthoritative_THEN_has_AuthorityLostRecord)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	// WHEN
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_AUTHORITATIVE);
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, Fixture.kTestComponentId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);

	// THEN
	Fixture.ExpectedAuthorityLost.Push(Fixture.kEntityComponentId);
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}

AUTHORITYRECORD_TEST(
	GIVEN_AuthorityRecord_with_AuthoritativeRecord_NonAuthoritativeRecord_and_AuthorityLostTemporarilyRecorde_WHEN_Cleared_THEN_has_no_records)
{
	// GIVEN
	AuthorityChangeRecordFixture Fixture;
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, 1, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, 2, WORKER_AUTHORITY_AUTHORITATIVE);
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, 3, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
	Fixture.Record.SetAuthority(Fixture.kTestEntityId, 3, WORKER_AUTHORITY_AUTHORITATIVE);
	Fixture.ExpectedAuthorityLost.Push(EntityComponentId{ Fixture.kTestEntityId, 1 });
	Fixture.ExpectedAuthorityGained.Push(EntityComponentId{ Fixture.kTestEntityId, 2 });
	Fixture.ExpectedAuthorityLostTemporarily.Push(EntityComponentId{ Fixture.kTestEntityId, 3 });
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	// WHEN
	Fixture.Record.Clear();

	// THEN
	Fixture.ExpectedAuthorityLost.Empty();
	Fixture.ExpectedAuthorityGained.Empty();
	Fixture.ExpectedAuthorityLostTemporarily.Empty();
	TestTrue(TEXT("Comparing AuthorityGained"), Fixture.Record.GetAuthorityGained() == Fixture.ExpectedAuthorityGained);
	TestTrue(TEXT("Comparing AuthorityLost"), Fixture.Record.GetAuthorityLost() == Fixture.ExpectedAuthorityLost);
	TestTrue(TEXT("Comparing AuthorityLostTemporarily"),
			 Fixture.Record.GetAuthorityLostTemporarily() == Fixture.ExpectedAuthorityLostTemporarily);

	return true;
}
