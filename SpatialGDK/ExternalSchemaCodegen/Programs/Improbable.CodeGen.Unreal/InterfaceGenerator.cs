using Improbable.Codegen.Base;
using Improbable.CodeGen.Base;
using System;
using System.Collections.Generic;
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

            builder.AppendLine($"#pragma once");
            builder.Append(Environment.NewLine);
            builder.AppendLine($"#include \"CoreMinimal.h\"");
            builder.AppendLine($"#include \"Engine/NetDriver.h\"");
            builder.AppendLine($"#include \"SpatialConstants.h\"");
            builder.AppendLine($"#include \"SpatialDispatcher.h\"");
            builder.AppendLine($"#include \"SpatialNetDriver.h\"");
            builder.Append(Environment.NewLine);
            builder.AppendLine($"#include \"{UnrealGenerator.RelativeIncludePrefix}HelperFunctions.h\"");
            foreach (var component in componentTypes)
            {
                builder.AppendLine($"#include \"{UnrealGenerator.RelativeIncludePrefix}{Types.TypeToHeaderFilename(component.QualifiedName)}\"");
            }
            builder.Append(Environment.NewLine);
            builder.AppendLine($"#include <WorkerSDK/improbable/c_worker.h>");
            builder.Append(Environment.NewLine);
            builder.AppendLine($"DECLARE_LOG_CATEGORY_EXTERN(LogExternalSchemaInterface, All, All);");
            builder.Append(Environment.NewLine);

            builder.AppendLine($"class {ClassName}");
            builder.AppendLine("{");
            builder.AppendLine("public:");
            builder.AppendLine(Text.Indent(1, $"{ClassName}() = delete;"));
            builder.AppendLine(Text.Indent(1, $"{ClassName}(USpatialWorkerConnection* Connection, USpatialDispatcher* SpatialDispatcher)"));
            builder.AppendLine(Text.Indent(2, $": SpatialWorkerConnection(SpatialWorkerConnection), SpatialDispatcher(SpatialDispatcher) {{ }}"));
            builder.AppendLine(Text.Indent(1, $"~{ClassName}() = default;"));
            builder.Append(Environment.NewLine);
            builder.AppendLine(Text.Indent(1, $"void RemoveCallback(USpatialDispatcher::FCallbackId Id);"));
            builder.Append(Environment.NewLine);

            foreach (var component in componentTypes)
            {
                var qualifiedTypeName = Types.GetQualifiedTypeFromQualifiedName(component.QualifiedName);
                builder.AppendLine(Text.Indent(1, $"// Component {component.QualifiedName} = {component.ComponentId}"));
                builder.AppendLine(Text.Indent(1, $"void SendComponentUpdate(Worker_EntityId EntityId, const {qualifiedTypeName}::Update& Update);"));
                builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnAddComponent(const TFunction<void(const {qualifiedTypeName}::AddComponentOp&)>& Callback);"));
                builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnRemoveComponent(const TFunction<void(const {qualifiedTypeName}::RemoveComponentOp&)>& Callback);"));
                builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnComponentUpdate(const TFunction<void(const {qualifiedTypeName}::ComponentUpdateOp&)>& Callback);"));
                builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnAuthorityChange(const TFunction<void(const {qualifiedTypeName}::AuthorityChangeOp&)>& Callback);"));

                if (bundle.Components[component.QualifiedName].CommandDefinitions.Count > 0)
                {
                    foreach (var command in bundle.Components[component.QualifiedName].CommandDefinitions)
                    {
                        var commandName = Text.SnakeCaseToPascalCase(command.Identifier.Name);
                        builder.Append(Environment.NewLine);
                        builder.AppendLine(Text.Indent(1, $"// command {commandName} = {component.ComponentId}"));
                        builder.AppendLine(Text.Indent(1, $"Worker_RequestId SendCommandRequest(Worker_EntityId EntityId, const {qualifiedTypeName}::Commands::{commandName}::Request& Request);"));
                        builder.AppendLine(Text.Indent(1, $"void SendCommandResponse(Worker_RequestId RequestId, const {qualifiedTypeName}::Commands::{commandName}::Response& Response);"));
                        builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnCommandRequest(const TFunction<void(const {qualifiedTypeName}::Commands::{commandName}::RequestOp&)>& Callback);"));
                        builder.AppendLine(Text.Indent(1, $"USpatialDispatcher::FCallbackId OnCommandResponse(const TFunction<void(const {qualifiedTypeName}::Commands::{commandName}::ResponseOp&)>& Callback);"));
                    }
                }
                builder.Append(Environment.NewLine);
            }

            builder.AppendLine($"private:");
            builder.AppendLine(Text.Indent(1, $@"void SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update);"));
            builder.AppendLine(Text.Indent(1, $@"Worker_RequestId SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request);"));
            builder.AppendLine(Text.Indent(1, $@"void SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response);"));
            builder.Append(Environment.NewLine);
            builder.AppendLine(Text.Indent(1, $@"USpatialWorkerConnection* SpatialWorkerConnection;"));
            builder.AppendLine(Text.Indent(1, $@"USpatialDispatcher* SpatialDispatcher;"));
            builder.AppendLine("};");

            return builder.ToString();
        }

        public static string GenerateInterfaceSource(List<TypeDescription> componentTypes, Bundle bundle)
        {
            var builder = new StringBuilder();

            builder.AppendLine($"#include \"{UnrealGenerator.RelativeIncludePrefix}/{HeaderPath}\"");
            builder.Append(Environment.NewLine);
            builder.AppendLine($"DEFINE_LOG_CATEGORY(LogExternalSchemaInterface);");
            builder.Append(Environment.NewLine);

            builder.AppendLine($"void {ClassName}::RemoveCallback(USpatialDispatcher::FCallbackId Id)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, "SpatialDispatcher->RemoveOpCallback(Id);"));
            builder.AppendLine("}");
            builder.Append(Environment.NewLine);

            builder.AppendLine($"void {ClassName}::SerializeAndSendComponentUpdate(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const ::improbable::SpatialComponentUpdate& Update)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, "Worker_ComponentUpdate SerializedUpdate = {};"));
            builder.AppendLine(Text.Indent(1, "SerializedUpdate.component_id = ComponentId;"));
            builder.AppendLine(Text.Indent(1, "SerializedUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);"));
            builder.AppendLine(Text.Indent(1, "Update.Serialize(SerializedUpdate.schema_type);"));
            builder.AppendLine(Text.Indent(1, "SpatialWorkerConnection->SendComponentUpdate(EntityId, &SerializedUpdate);"));
            builder.AppendLine("}");
            builder.Append(Environment.NewLine);

            builder.AppendLine($"Worker_RequestId {ClassName}::SerializeAndSendCommandRequest(Worker_EntityId EntityId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Request)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, "Worker_CommandRequest SerializedRequest = {};"));
            builder.AppendLine(Text.Indent(1, "SerializedRequest.component_id = ComponentId;"));
            builder.AppendLine(Text.Indent(1, "SerializedRequest.schema_type = Schema_CreateCommandRequest(ComponentId, CommandIndex);"));
            builder.AppendLine(Text.Indent(1, "Schema_Object* RequestObject = Schema_GetCommandRequestObject(SerializedRequest.schema_type);"));
            builder.AppendLine(Text.Indent(1, "Request.Serialize(RequestObject);"));
            builder.AppendLine(Text.Indent(1, "return SpatialWorkerConnection->SendCommandRequest(EntityId, &SerializedRequest, CommandIndex);"));
            builder.AppendLine("}");
            builder.Append(Environment.NewLine);

            builder.AppendLine($"void {ClassName}::SerializeAndSendCommandResponse(Worker_RequestId RequestId, Worker_ComponentId ComponentId, Schema_FieldId CommandIndex, const ::improbable::SpatialType& Response)");
            builder.AppendLine("{");
            builder.AppendLine(Text.Indent(1, "Worker_CommandResponse SerializedResponse = {};"));
            builder.AppendLine(Text.Indent(1, "SerializedResponse.component_id = ComponentId;"));
            builder.AppendLine(Text.Indent(1, "SerializedResponse.schema_type = Schema_CreateCommandResponse(ComponentId, CommandIndex);"));
            builder.AppendLine(Text.Indent(1, "Schema_Object* ResponseObject = Schema_GetCommandResponseObject(SerializedResponse.schema_type);"));
            builder.AppendLine(Text.Indent(1, "Response.Serialize(ResponseObject);"));
            builder.AppendLine(Text.Indent(1, "return SpatialWorkerConnection->SendCommandResponse(RequestId, &SerializedResponse);"));
            builder.AppendLine("}");
            builder.Append(Environment.NewLine);

            foreach (var component in componentTypes)
            {
                var qualifiedTypeName = Types.GetQualifiedTypeFromQualifiedName(component.QualifiedName);
                builder.AppendLine( $"// Component {component.QualifiedName} = {component.ComponentId}");

                builder.AppendLine($"void {ClassName}::SendComponentUpdate(const Worker_EntityId EntityId, const {qualifiedTypeName}::Update& Update)");
                builder.AppendLine("{");
                builder.AppendLine(Text.Indent(1, $"SerializeAndSendComponentUpdate(EntityId, {component.ComponentId}, Update);"));
                builder.AppendLine("}");
                builder.Append(Environment.NewLine);

                builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnAddComponent(const TFunction<void(const {qualifiedTypeName}::AddComponentOp&)>& Callback)");
                builder.AppendLine("{");
                builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnAddComponent({component.ComponentId}, [&](Worker_AddComponentOp Op)"));
                builder.AppendLine(Text.Indent(1, "{"));
                builder.AppendLine(Text.Indent(2, $"{qualifiedTypeName} Data = {qualifiedTypeName}::Deserialize(Op.data.schema_type);"));
                builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::AddComponentOp(Op.entity_id, Op.data.component_id, Data));"));
                builder.AppendLine(Text.Indent(1, "});"));
                builder.AppendLine("}");
                builder.Append(Environment.NewLine);

                builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnRemoveComponent(const TFunction<void(const {qualifiedTypeName}::RemoveComponentOp&)>& Callback)");
                builder.AppendLine("{");
                builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnRemoveComponent({component.ComponentId}, [&](Worker_RemoveComponentOp Op)"));
                builder.AppendLine(Text.Indent(1, "{"));
                builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::RemoveComponentOp(Op.entity_id, Op.component_id));"));
                builder.AppendLine(Text.Indent(1, "});"));
                builder.AppendLine("}");
                builder.Append(Environment.NewLine);

                builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnComponentUpdate(const TFunction<void(const {qualifiedTypeName}::ComponentUpdateOp&)>& Callback)");
                builder.AppendLine("{");
                builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnComponentUpdate({component.ComponentId}, [&](Worker_ComponentUpdateOp Op)"));
                builder.AppendLine(Text.Indent(1, "{"));
                builder.AppendLine(Text.Indent(2, $"{qualifiedTypeName}::Update Update = {qualifiedTypeName}::Update::Deserialize(Op.update.schema_type);"));
                builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::ComponentUpdateOp(Op.entity_id, Op.update.component_id, Update));"));
                builder.AppendLine(Text.Indent(1, "});"));
                builder.AppendLine("}");
                builder.Append(Environment.NewLine);

                builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnAuthorityChange(const TFunction<void(const {qualifiedTypeName}::AuthorityChangeOp&)>& Callback)");
                builder.AppendLine("{");
                builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnAuthorityChange({component.ComponentId}, [&](Worker_AuthorityChangeOp Op)"));
                builder.AppendLine(Text.Indent(1, "{"));
                builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::AuthorityChangeOp(Op.entity_id, Op.component_id, static_cast<Worker_Authority>(Op.authority)));"));
                builder.AppendLine(Text.Indent(1, "});"));
                builder.AppendLine("}");

                if (bundle.Components[component.QualifiedName].CommandDefinitions.Count > 0)
                {
                    foreach (var command in bundle.Components[component.QualifiedName].CommandDefinitions)
                    {
                        var commandName = Text.SnakeCaseToPascalCase(command.Identifier.Name);
                        builder.Append(Environment.NewLine);
                        builder.AppendLine($"// command {commandName} = {component.ComponentId}");
                        builder.Append(Environment.NewLine);

                        builder.AppendLine($"Worker_RequestId {ClassName}::SendCommandRequest(Worker_EntityId EntityId, const {qualifiedTypeName}::Commands::{commandName}::Request& Request)");
                        builder.AppendLine("{");
                        builder.AppendLine(Text.Indent(1, $"return SerializeAndSendCommandRequest(EntityId, {component.ComponentId}, {command.CommandIndex}, Request.Data);"));
                        builder.AppendLine("}");
                        builder.Append(Environment.NewLine);

                        builder.AppendLine($"void {ClassName}::SendCommandResponse(Worker_RequestId RequestId, const {qualifiedTypeName}::Commands::{commandName}::Response& Response)");
                        builder.AppendLine("{");
                        builder.AppendLine(Text.Indent(1, $"SerializeAndSendCommandResponse(RequestId, {component.ComponentId}, {command.CommandIndex}, Response.Data);"));
                        builder.AppendLine("}");
                        builder.Append(Environment.NewLine);

                        builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnCommandRequest(const TFunction<void(const {qualifiedTypeName}::Commands::{commandName}::RequestOp&)>& Callback)");
                        builder.AppendLine("{");
                        builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnCommandRequest({component.ComponentId}, [&](Worker_CommandRequestOp Op)"));
                        builder.AppendLine(Text.Indent(1, "{"));
                        builder.AppendLine(Text.Indent(2, $"auto Request = {qualifiedTypeName}::Commands::{commandName}::Request({qualifiedTypeName}::Commands::{commandName}::Request::Type::Deserialize(Schema_GetCommandRequestObject(Op.request.schema_type)));"));
                        builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::Commands::{commandName}::RequestOp(Op.entity_id, Op.request_id, Op.timeout_millis, Op.caller_worker_id, Op.caller_attribute_set, Request));"));
                        builder.AppendLine(Text.Indent(1, "});"));
                        builder.AppendLine("}");
                        builder.Append(Environment.NewLine);

                        builder.AppendLine($"USpatialDispatcher::FCallbackId {ClassName}::OnCommandResponse(const TFunction<void(const {qualifiedTypeName}::Commands::{commandName}::ResponseOp&)>& Callback)");
                        builder.AppendLine("{");
                        builder.AppendLine(Text.Indent(1, $"return SpatialDispatcher->OnCommandResponse({component.ComponentId}, [&](Worker_CommandResponseOp Op)"));
                        builder.AppendLine(Text.Indent(1, "{"));
                        builder.AppendLine(Text.Indent(2, $"auto Response = {qualifiedTypeName}::Commands::{commandName}::Response({qualifiedTypeName}::Commands::{commandName}::Response::Type::Deserialize(Schema_GetCommandResponseObject(Op.response.schema_type)));"));
                        builder.AppendLine(Text.Indent(2, $"Callback({qualifiedTypeName}::Commands::{commandName}::ResponseOp(Op.entity_id, Op.request_id, Op.status_code, Op.message, Op.command_id, Response));"));
                        builder.AppendLine(Text.Indent(1, "});"));
                        builder.AppendLine("}");
                    }
                }
            }

            return builder.ToString();
        }
    }
}
