using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Improbable.CodeGen.Unreal
{
    public static class InterfaceGenerator
    {
        public static string ClassName = $"ExternalSchemaInterface";
        public static string HeaderPath = $"ExternalSchemaInterface.h";
        public static string SourceFile = $"ExternalSchemaInterface.cpp";

        public static List<GeneratedFile> GenerateInterface(List<TypeDescription> componentTypes, Bundle bundle)
        {
            return new List<GeneratedFile>
            {
                new GeneratedFile(HeaderPath, GenerateInterfaceHeader(componentTypes, bundle)),
                new GeneratedFile(SourceFile, GenerateInterfaceSource(componentTypes, bundle))
            };
        }

        public static string GenerateInterfaceHeader(List<TypeDescription> componentTypes, Bundle bundle)
        {
            var builder = new StringBuilder();

            builder.AppendLine($@"#pragma once

#include ""CoreMinimal.h""
#include ""SpatialConstants.h""
#include ""SpatialDispatcher.h""
#include ""{HelperFunctions.HeaderPath}""

{string.Join(Environment.NewLine, componentTypes.Select(component => $"#include \"{Types.TypeToHeaderFilename(component.QualifiedName)}\""))}

#include <WorkerSDK/improbable/c_worker.h>

class USpatialWorkerConnection;

DECLARE_LOG_CATEGORY_EXTERN(LogExternalSchemaInterface, Log, All);

class {ClassName}
{{
public:
{Text.Indent(1, $@"{ClassName} () = delete;
{ClassName}(USpatialWorkerConnection* Connection, USpatialDispatcher* Dispatcher)
{Text.Indent(1, @": SpatialWorkerConnection(Connection), SpatialDispatcher(Dispatcher) {{ }}")}
~{ClassName}() = default;

void RemoveCallback(USpatialDispatcher::FCallbackId Id);
")}");
            foreach (var component in componentTypes)
            {
                var qualifiedType = Types.GetTypeDisplayName(component.QualifiedName);
                builder.AppendLine(Text.Indent(1, $@"// Component {component.QualifiedName} = {component.ComponentId}
void SendComponentUpdate(Worker_EntityId EntityId, const {qualifiedType}::Update& Update);
USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const {qualifiedType}::AddComponentOp&)>& Callback);
USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const {qualifiedType}::RemoveComponentOp&)>& Callback);
USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const {qualifiedType}::ComponentUpdateOp&)>& Callback);
USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const {qualifiedType}::AuthorityChangeOp&)>& Callback);
"));
                if (bundle.Components[component.QualifiedName].Commands.Count() > 0)
                {
                    builder.AppendLine(Text.Indent(1, string.Join(Environment.NewLine, bundle.Components[component.QualifiedName].Commands.Select(command => $@"// command {Text.SnakeCaseToPascalCase(command.Name)} = {component.ComponentId}
Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Request& Request);
void SendCommandResponse(Worker_RequestId RequestId, const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Response& Response);
USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::RequestOp&)>& Callback);
USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::ResponseOp&)>& Callback);
"))));
                }
            }

            builder.AppendLine($@"private:
{Text.Indent(1, $@"void SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update);
Worker_RequestId SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request);
void SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response);

USpatialWorkerConnection* SpatialWorkerConnection;
USpatialDispatcher* SpatialDispatcher;")}
}};");

            return builder.ToString();
        }

        public static string GenerateInterfaceSource(List<TypeDescription> componentTypes, Bundle bundle)
        {
            return $@"#include ""{HeaderPath}""
#include ""Connection/SpatialWorkerConnection.h""

DEFINE_LOG_CATEGORY(LogExternalSchemaInterface);

void {ClassName}::RemoveCallback(USpatialDispatcher::FCallbackId Id)
{{
{Text.Indent(1, $"SpatialDispatcher->RemoveOpCallback(Id);")}
}}

void {ClassName}::SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update)
{{
{Text.Indent(1, $@"Worker_ComponentUpdate SerializedUpdate = {{}};
SerializedUpdate.component_id = ComponentId;
SerializedUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
Update.Serialize(SerializedUpdate.schema_type);
SpatialWorkerConnection->SendComponentUpdate(EntityId, &SerializedUpdate);")}
}}

Worker_RequestId {ClassName}::SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request)
{{
{Text.Indent(1, $@"Worker_CommandRequest SerializedRequest = {{}};
SerializedRequest.component_id = ComponentId;
SerializedRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);
Schema_Object* RequestObject = Schema_GetCommandRequestObject(SerializedRequest.schema_type);
Request.Serialize(RequestObject);
return SpatialWorkerConnection->SendCommandRequest(EntityId, &SerializedRequest, CommandIndex);")}
}}

void {ClassName}::SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response)
{{
{Text.Indent(1, $@"Worker_CommandResponse SerializedResponse = {{}};
SerializedResponse.component_id = ComponentId;
SerializedResponse.schema_type = Schema_CreateCommandResponse(ComponentId, CommandIndex);
Schema_Object* ResponseObject = Schema_GetCommandResponseObject(SerializedResponse.schema_type);
Response.Serialize(ResponseObject);
return SpatialWorkerConnection->SendCommandResponse(RequestId, &SerializedResponse);")}
}}

{string.Join(Environment.NewLine, componentTypes.Select(component => $@"// Component {component.QualifiedName} = {component.ComponentId}
void {ClassName}::SendComponentUpdate(const Worker_EntityId EntityId, const {Types.GetTypeDisplayName(component.QualifiedName)}::Update& Update)
{{             
{Text.Indent(1, $"SerializeAndSendComponentUpdate(EntityId, {component.ComponentId}, Update);")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnAddComponent(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::AddComponentOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnAddComponent({component.ComponentId}, [Callback](const Worker_AddComponentOp& Op)
{{
{Text.Indent(1, $@"{Types.GetTypeDisplayName(component.QualifiedName)} Data = {Types.GetTypeDisplayName(component.QualifiedName)}::Deserialize(Op.data.schema_type);
Callback({Types.GetTypeDisplayName(component.QualifiedName)}::AddComponentOp(Op.entity_id, Op.data.component_id, Data));")}
}});")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnRemoveComponent(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::RemoveComponentOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnRemoveComponent({component.ComponentId}, [Callback](const Worker_RemoveComponentOp& Op)
{{
{Text.Indent(1, $"Callback({Types.GetTypeDisplayName(component.QualifiedName)}::RemoveComponentOp(Op.entity_id, Op.component_id));")}
}});")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnComponentUpdate(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::ComponentUpdateOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnComponentUpdate({component.ComponentId}, [Callback](const Worker_ComponentUpdateOp& Op)
{{
{Text.Indent(1, $@"{Types.GetTypeDisplayName(component.QualifiedName)}::Update Update = {Types.GetTypeDisplayName(component.QualifiedName)}::Update::Deserialize(Op.update.schema_type);
Callback({Types.GetTypeDisplayName(component.QualifiedName)}::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));")}
}});")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnAuthorityChange(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::AuthorityChangeOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnAuthorityChange({component.ComponentId}, [Callback](const Worker_AuthorityChangeOp& Op)
{{
{Text.Indent(1, $"Callback({Types.GetTypeDisplayName(component.QualifiedName)}::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));")}
}});")}
}}

{string.Join(Environment.NewLine, bundle.Components[component.QualifiedName].Commands.Select(command => $@"Worker_RequestId {ClassName}::SendCommandRequest(Worker_EntityId EntityId, const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{ Text.SnakeCaseToPascalCase(command.Name)}::Request& Request)
{{
{Text.Indent(1, $"return SerializeAndSendCommandRequest(EntityId, {component.ComponentId}, {command.CommandIndex}, Request.Data);")}
}}

void {ClassName}::SendCommandResponse(Worker_RequestId RequestId, const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Response& Response)
{{
{Text.Indent(1, $"SerializeAndSendCommandResponse(RequestId, {component.ComponentId}, {command.CommandIndex}, Response.Data);")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnCommandRequest(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{ Text.SnakeCaseToPascalCase(command.Name)}::RequestOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnCommandRequest({component.ComponentId}, [Callback](const Worker_CommandRequestOp& Op)
{{
{Text.Indent(1, $@"auto Request = {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Request({Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{ Text.SnakeCaseToPascalCase(command.Name)}::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));
Callback({Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));")}
}});")}
}}

USpatialDispatcher::FCallbackId {ClassName}::OnCommandResponse(const TFunction<void(const {Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::ResponseOp&)>& Callback)
{{
{Text.Indent(1, $@"return SpatialDispatcher->OnCommandResponse({component.ComponentId}, [Callback](const Worker_CommandResponseOp& Op)
{{
{Text.Indent(1, $@"if (Op.command_id == {command.CommandIndex})
{{
{Text.Indent(1, $@"auto Response = Op.status_code == Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS  ?
{Text.Indent(1, $@"{Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Response({Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{ Text.SnakeCaseToPascalCase(command.Name)}::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type))) :
{Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::Response();")}
Callback({Types.GetTypeDisplayName(component.QualifiedName)}::Commands::{Text.SnakeCaseToPascalCase(command.Name)}::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));")}
}}")}
}});")}
}}
"))}"))}";
        }
    }
}
