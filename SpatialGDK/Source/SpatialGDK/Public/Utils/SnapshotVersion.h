// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// SPATIAL_SNAPSHOT_VERSION is the current version of supported snapshots.
//
// If changes are made to the schema or content of snapshots then:
//	(a) (optional) if the schema has changed, update SPATIAL_SNAPSHOT_SCHEMA_HASH,
//			see the test
//'GIVEN_snapshot_affecting_schema_files_WHEN_hash_of_file_contents_is_generated_THEN_hash_matches_expected_snapshot_version_hash'
// which will provide the hash on failure
//  (b) (mandatory) increment SPATIAL_SNAPSHOT_VERSION_INC

constexpr uint32 SPATIAL_SNAPSHOT_SCHEMA_HASH = 944243238;
constexpr uint32 SPATIAL_SNAPSHOT_VERSION_INC = 1;

constexpr uint64 SPATIAL_SNAPSHOT_VERSION = ((((uint64)SPATIAL_SNAPSHOT_SCHEMA_HASH) << 32) | SPATIAL_SNAPSHOT_VERSION_INC);
