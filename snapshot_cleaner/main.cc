#include "c_worker.h"
#include "c_schema.h"
#include <stdio.h>
#include <fstream>
#include <vector>
#include <map>
#include <iterator>

const uint32_t PERSISTENCE_COMPONENT_ID = 9950;
const uint32_t MIN_GENERATED_COMPONENT_ID = 10000;

void Cleanup(Worker_SnapshotInputStream* ModifiedInStream, Worker_SnapshotOutputStream* OutStream, std::map<Worker_ComponentId, Worker_ComponentData*>* CleanData)
{
	Worker_SnapshotInputStream_Destroy(ModifiedInStream);
	Worker_SnapshotOutputStream_Destroy(OutStream);

	if (CleanData != nullptr)
	{
		for (auto it = CleanData->begin(); it != CleanData->end(); ++it)
		{
			Worker_ReleaseComponentData(it->second);
		}
	}
}

bool CheckInputError(Worker_SnapshotInputStream* Stream)
{
	Worker_SnapshotState State = Worker_SnapshotInputStream_GetState(Stream);
	if (State.stream_state == WORKER_STREAM_STATE_BAD)
	{
		printf("Input stream error'd: %s\n", State.error_message);
		return true;
	}

	return false;
}

bool CheckOutputError(Worker_SnapshotOutputStream* Stream)
{
	Worker_SnapshotState State = Worker_SnapshotOutputStream_GetState(Stream);
	if (State.stream_state == WORKER_STREAM_STATE_BAD)
	{
		printf("Input stream error'd: %s\n", State.error_message);
		return true;
	}

	return false;
}

bool HasCustomPersistence(const Worker_Entity* Entity)
{
	for (uint32_t CompIdx = 0; CompIdx < Entity->component_count; CompIdx++)
	{
		const Worker_ComponentData* CompData = &Entity->components[CompIdx];
		if (CompData->component_id == PERSISTENCE_COMPONENT_ID)
		{
			return true;
		}
	}
	return false;
}

bool ShouldCleanComponent(Worker_ComponentId Id)
{
	return (Id >= MIN_GENERATED_COMPONENT_ID);
}



bool ExtractCleanData(Worker_SnapshotInputStream* InStream, std::map<Worker_ComponentId, Worker_ComponentData*>& CleanData)
{
	while(Worker_SnapshotInputStream_HasNext(InStream))
	{
		const Worker_Entity* Entity = Worker_SnapshotInputStream_ReadEntity(InStream);
		if (CheckInputError(InStream))
		{
			return false;
		}

		if (HasCustomPersistence(Entity))
		{
			for (uint32_t CompIdx = 0; CompIdx < Entity->component_count; CompIdx++)
			{
				const Worker_ComponentData* CompData = &Entity->components[CompIdx];
				if (ShouldCleanComponent(CompData->component_id)) // this is a generated project-level component
				{
					Worker_ComponentData* DataCopy = Worker_AcquireComponentData(CompData);
					CleanData[CompData->component_id] = DataCopy;
				}
			}

				// We assume that the clean data for all entities with custom persistence is the same, so we stop iterating after we've found one entity with custom persistence
			return true;
		}
	}

	return true;
}

int main(int argc, char *argv[])
{
	if (argc <= 2) {
		printf("Need arguments: clean.snapshot modified.snapshot");
		return 1;
	}
		// Load our clean input data and extract the "default state" of the components we care about
	const Worker_ComponentVtable vtable = {};
	Worker_SnapshotParameters params;
	params.default_component_vtable = &vtable;
	params.snapshot_type = WORKER_SNAPSHOT_TYPE_BINARY;
	params.component_vtables = nullptr;
	params.component_vtable_count = 0;

	Worker_SnapshotInputStream* CleanInStream = Worker_SnapshotInputStream_Create(argv[1], &params);
	if (CheckInputError(CleanInStream))
	{
		Worker_SnapshotInputStream_Destroy(CleanInStream);
		return 1;
	}

	std::map<Worker_ComponentId, Worker_ComponentData*> CleanData;
	bool ExtractedCleanData = ExtractCleanData(CleanInStream, CleanData);
	Worker_SnapshotInputStream_Destroy(CleanInStream);
	if (!ExtractedCleanData)
	{
		return 1;
	}

		// Open modified input stream and output stream
	Worker_SnapshotInputStream* ModifiedInStream = Worker_SnapshotInputStream_Create(argv[2], &params);
	if (CheckInputError(ModifiedInStream))
	{
		Cleanup(ModifiedInStream, nullptr, &CleanData);
		return 1;
	}

	Worker_SnapshotOutputStream* OutStream = Worker_SnapshotOutputStream_Create("out.snapshot", &params);
	if (CheckOutputError(OutStream))
	{
		Cleanup(ModifiedInStream, OutStream, &CleanData);
		return 1;
	}

	int CleanedComponentCount = 0;

		// look at each component
		// if it has custom persistence, overwrite the components we care about with the clean data we grabbed from the clean snapshot
	while(Worker_SnapshotInputStream_HasNext(ModifiedInStream))
	{
		const Worker_Entity* Entity = Worker_SnapshotInputStream_ReadEntity(ModifiedInStream);
		if (CheckInputError(ModifiedInStream))
		{
			Cleanup(ModifiedInStream, OutStream, &CleanData);
			return 1;
		}

		if (HasCustomPersistence(Entity))
		{
			CleanedComponentCount++;

			std::vector<Worker_ComponentData> NewEntityComponentDatas;

			for (uint32_t CompIdx = 0; CompIdx < Entity->component_count; CompIdx++)
			{
				const Worker_ComponentData* CompData = &Entity->components[CompIdx];

				if (CleanData.count(CompData->component_id) == 1)
				{
					const Worker_ComponentData* CleanCompData = CleanData.at(CompData->component_id);
					NewEntityComponentDatas.push_back(*CleanCompData);
				}
				else
				{
					NewEntityComponentDatas.push_back(*CompData);
				}
			}

			// Construct a new entity and write it out
			Worker_Entity NewEntity = {
				Entity->entity_id,
				static_cast<uint32_t>(NewEntityComponentDatas.size()),
				NewEntityComponentDatas.data() };

			Worker_SnapshotOutputStream_WriteEntity(OutStream, &NewEntity);
		}
		else
		{
			// If it doesn't have persistent data to clean out, write the entity out as it is
			Worker_SnapshotOutputStream_WriteEntity(OutStream, Entity);
		}
		
		if (CheckOutputError(OutStream))
		{
			Cleanup(ModifiedInStream, OutStream, &CleanData);
			return 1;
		}
	}

	printf("Cleaned %d components\n", CleanedComponentCount);

	Cleanup(ModifiedInStream, OutStream, &CleanData);

	return 0;
}


// Reference code for loading a schema bundle and using it to print JSON-serialised component data:

// Load a schema bundle:
// Note that this takes the binary schema bundle, i.e. the file with .sb extension

// std::ifstream BundleInStream(argv[3], std::ios::binary);
// std::vector<char> SchemaBundleData(std::istreambuf_iterator<char>(BundleInStream), {});
// BundleInStream.close();

// Schema_Bundle* SchemaBundle = Schema_Bundle_Load((uint8_t*)SchemaBundleData.data(), SchemaBundleData.size());
// if (const char* BundleLoadError = Schema_Bundle_GetError(SchemaBundle))
// {
// 	printf("Bundle load error: %s\n", BundleLoadError);
// 	Cleanup(nullptr, nullptr, SchemaBundle, nullptr);
// 	return 1;
// }

// Don't forget to destroy it with Schema_Bundle_Destroy(SchemaBundle);

// Printing component data:

// bool PrintComponentData(Schema_Bundle* SchemaBundle, const Worker_ComponentData* Data)
// {
// 	Schema_Json* ComponentDataJson = Schema_Json_DumpComponentData(SchemaBundle, Data->component_id, Data->schema_type);
// 	if (ComponentDataJson == nullptr)
// 	{
// 		const char* JsonError = Schema_Json_GetLastError();
// 		printf("Failed to dump component %u as Json: %s\n", Data->component_id, JsonError);
// 		return false;
// 	}

// 	const char* JsonString = Schema_Json_GetJsonString(ComponentDataJson);
// 	printf("Component %u, Json: %s\n", Data->component_id, JsonString);

// 	return true;
// }