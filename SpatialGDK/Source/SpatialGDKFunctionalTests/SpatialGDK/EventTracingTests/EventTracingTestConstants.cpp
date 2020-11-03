// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingTestConstants.h"

const FName UEventTracingTestConstants::ReceiveOpEventName = "worker.receive_op";
const FName UEventTracingTestConstants::SendPropertyUpdatesEventName = "unreal_gdk.send_property_updates";
const FName UEventTracingTestConstants::ReceivePropertyUpdateEventName = "unreal_gdk.receive_property_update";
const FName UEventTracingTestConstants::SendRPCEventName = "unreal_gdk.send_rpc";
const FName UEventTracingTestConstants::ProcessRPCEventName = "unreal_gdk.process_rpc";
const FName UEventTracingTestConstants::ComponentUpdateEventName = "unreal_gdk.component_update";
const FName UEventTracingTestConstants::MergeComponentUpdateEventName = "unreal_gdk.merge_component_update";
const FName UEventTracingTestConstants::UserProcessRPCEventName = "user.process_rpc";
const FName UEventTracingTestConstants::UserReceivePropertyEventName = "user.receive_property";
const FName UEventTracingTestConstants::UserReceiveComponentPropertyEventName = "user.receive_component_property";
const FName UEventTracingTestConstants::UserSendPropertyEventName = "user.send_property";
const FName UEventTracingTestConstants::UserSendComponentPropertyEventName = "user.send_component_property";
const FName UEventTracingTestConstants::UserSendRPCEventName = "user.send_rpc";
