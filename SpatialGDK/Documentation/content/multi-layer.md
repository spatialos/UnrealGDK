# Multi-layer development
<%(TOC)%>

Having multiple layers in a SpatialOS game means using multiple worker types to simulate different aspects of the same deployment. For example, your game could use Unreal servers workers developed with the GDK *and* a separate Python Database worker built using the [C API](https://docs.improbable.io/reference/latest/capi/introduction) in coordination with a 3rd party service. This lets you modularize game elements, and reuse these separate worker types across multiple games.

In order to interact with other layers, the Unreal GDK needs to send data, receive data, and edit snapshots, for components defined in external schema files. This functionality is made possible through direct use of the C API within the Unreal GDK.

## How multi-layer is implemented

Sending data directly via the C API can be achieved using methods defined in the `SpatialWorkerConnection` (described in the section below). Receiving network ops for externally defined Schema components, which do not have Unreal component equivalents, is not handled by default as part of GDK pipeline. Instead, you can provide custom callbacks for specific component IDs and ops, and the GDK pipeline forwards the ops to your methods. Similarly, you can provide custom snapshot generation functions letting you serialize and add additional entities to the Snapshot output stream, which runs when the `Snapshot` button in the toolbar is clicked.

## How to use multi-layer

### Sending data

You can send component updates, command requests, and command responses directly to the C API using the `SpatialWorkerConnection.h` public methods:

```
void SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate);
Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId);
void SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response);
```

which are accessible via a reference to the net driver:

```
SpatialWorkerConnection* Connection = Cast<USpatialNetDriver>(World->GetNetDriver())->Connection;
```

There is a basic example in the section below, and for more examples of how to construct component updates, command requests, and more, check the [C API page on serialization](https://docs.improbable.io/reference/latest/capi/serialization).

### Receiving data

You can receive components updates through implementing dispatcher callbacks for specified component IDs and op types. These callbacks are detected by the Unreal dispatcher pipeline on initialization and used instead of the existing pipeline where the component ID and op type are matched.

To do this, you need to create a class derived from the `UOpCallbackTemplate` base class and implement the `GetComponentId` and one or more of the callback methods from the. These callback methods are parameterized by C API op types:

```
virtual void OnAddComponent(const Worker_AddComponentOp& op) {}
virtual void OnRemoveComponent(const Worker_RemoveComponentOp& op) {}
virtual void OnComponentUpdate(const Worker_ComponentUpdateOp& op) {}
virtual void OnAuthorityChange(const Worker_AuthorityChangeOp& op) {}
virtual void OnCommandRequest(const Worker_CommandRequestOp& op) {}
virtual void OnCommandResponse(const Worker_CommandResponseOp& op) {}
  ```

There is a basic example in the section below, and for more examples of how to deserialize C API types, check the [C API page on serialization](https://docs.improbable.io/reference/latest/capi/serialization).

### Adding to snapshots

You can customize snapshot generation through creating a class derived from the `USnapshotGenerationTemplate` base class, and implementing the following method:

```
/**
	* Write to the snapshot generation output stream
	* @param OutputStream the output stream for the snapshot being created.
	* @param NextEntityId the next available entity ID in the snapshot, this reference should be incremented appropriately.
	* @return bool the success of writing to the snapshot output stream, this is returned to the overall snapshot generation.
	**/
bool WriteToSnapshotOutput(Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextEntityId);
```

## Example

Below is a simple, example schema file which could be used by a separate worker layer to track player statistics:

```
package improbable.session;

type MyRequest {
  string player_name = 1;
}
type MyResponse {}

component Session {
    id = 1337;
    uint32 player_count = 1;
    command MyResponse some_command(My_Request);
}
```

#### Sending data

You could serialize and send a component update in user code in the following way:

```
void SendSomeUpdate(Worker_EntityId target_entity_id, Worker_ComponentId component_id) {
    Worker_ComponentUpdate component_update;
    component_update.id = component_id
    component_update.schema_type = Schema_CreateComponentUpdate(component_id);

    //  Component specific serialization
    Schema_Object* fields_object = Schema_GetComponentUpdateFields(component_update.schema_type);
    Schema_AddInt32(fields_object, 1, 1234);

    Cast<USpatialNetDriver>(World->GetNetDriver())->Connection->SendComponentUpdate(target_entity_id, component_update);
}
```

You could serialize and send a command request in user code in the following way:

```
Worker_RequestId SendSomeCommandRequest(Worker_EntityId target_entity_id, Worker_ComponentId component_id, uint32_t command_id, char* player_name) {
    Worker_CommandRequest command_request;
    command_request.component_id = component_id;
    command_request.schema_type = Schema_CreateCommandRequest(component_id, command_id);
    Schema_Object* request_object = Schema_GetCommandRequestObject(command_request.schema_type);

    // Command specific serialization
    Schema_AddBytes(request_object, 1, (const uint8_t*)player_name, sizeof(char) * strlen(player_name));

    Worker_RequestId SendCommandRequest(target_entity_id, command_request, command_id);
}
```

#### Receiving data

You could receive and deserialize a component update and command request in user code in the following way:

```
UCLASS()
class SPATIALGDK_API SessionComponentCallbacks : UOpCallbackTemplate
{
    GENERATED_BODY()

public:
    Worker_ComponentId GetComponentId() {
        return 1337;
    }

    void OnComponentUpdate(const Worker_ComponentUpdateOp& op) override {
        Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(op.schema_type);

        uint32_t player_count;
        if (Schema_GetObjectCount(ComponentObject, component_id) > 0)
        {
            if (Schema_GetInt32Count(ComponentObject, 1)) {
                player_count = Schema_GetInt32(ComponentObject, 1);
            }
        }

        // do something with player_count
    }

    void OnCommandRequest(const Worker_CommandRequestOp& op) override {
        Schema_Object* request_object = Schema_GetCommandRequestObject(op.request.schema_type);

        uint32_t player_name_length = Schema_GetBytesLength(fields_object, 1);
        const uint8_t* player_name_buffer = Schema_GetBytes(fields_object, 1);
        char* player_name = (char*) malloc(player_name_length + 1); // include space for null terminator
        memcpy(player_name, player_name_buffer, player_name_length);
        player_name[player_name_length] = '\0'; // add null terminator

        // do something with player_name

        // and send command response
    }
}
```

#### Editing the snapshot

You could add a new entity with the given component in user code in the following way:

```
UCLASS()
class SPATIALGDK_API USessionEntitySnapshotGeneration : USnapshotGenerationTemplate
{
    GENERATED_BODY()

public:
    bool WriteToSnapshotOutput(Worker_SnapshotOutputStream* OutputStream, Worker_Entity& NextEntityId) override {
        Worker_Entity SessionEntity;
        SessionEntity.entity_id = NextEntityId;

        TArray<Worker_ComponentData> Components;

        const WorkerAttributeSet SessionWorkerAttributeSet{ TArray<FString>{TEXT("SessionWorker")} };
        const WorkerRequirementSet SessionWorkerPermission{ SessionWorkerAttributeSet };
        const WorkerRequirementSet AnyWorkerPermission{ {UnrealClientAttributeSet, UnrealServerAttributeSet, SessionWorkerAttributeSet } };

        WriteAclMap ComponentWriteAcl;
        ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, UnrealServerPermission);
        ComponentWriteAcl.Add(SpatialConstants::METADATA_COMPONENT_ID, UnrealServerPermission);
        ComponentWriteAcl.Add(SpatialConstants::PERSISTENCE_COMPONENT_ID, UnrealServerPermission);
        ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, UnrealServerPermission);
        ComponentWriteAcl.Add(1337, SessionWorkerPermission);

        // Serialize Session component data
        Worker_ComponentData SessionComponentData{};
        SessionComponentData.component_id = 1337;
        SessionComponentData.schema_type = Schema_CreateComponentData(1337);
        Schema_Object* SessionComponentDataObject = Schema_GetComponentDataFields(SessionComponentData.schema_type);
        Schema_AddInt32(SessionComponentDataObject, 1, 0); // set player_count to 0 initially

        Components.Add(improbable::Position(Origin).CreatePositionData());
        Components.Add(improbable::Metadata(TEXT("SessionManagerEntity")).CreateMetadataData());
        Components.Add(improbable::Persistence().CreatePersistenceData());
        Components.Add(improbable::EntityAcl(AnyWorkerPermission, ComponentWriteAcl).CreateEntityAclData());
        Components.Add(SessionComponentData);

        SessionEntity.component_count = Components.Num();
        SessionEntity.components = Components.GetData();

        bool success = Worker_SnapshotOutputStream_WriteEntity(OutputStream, &SessionEntity) != 0;
        if (success) {
            NextEntityId++;
        }

        return success;
    }
}
```
