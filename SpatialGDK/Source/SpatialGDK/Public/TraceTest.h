// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// #include <cassert>
// #include <chrono>
// #include <thread>
// 
// #define WIN32_LEAN_AND_MEAN
// #define NOMINMAX
// #include "opencensus/trace/span.h"
// #include "opencensus/exporters/trace/stdout/stdout_exporter.h"
// #include "opencensus/exporters/trace/stackdriver/stackdriver_exporter.h"
// 
// void TestTrace()
// {
//  	using namespace ::opencensus::trace;
//  	using namespace ::opencensus::exporters::trace;
//  	// opencensus::exporters::trace::StdoutExporter::Register();
//  	StackdriverOptions options{};
//  	options.project_id = "your-gcpprojectid-123456";
//  	StackdriverExporter::Register(std::move(options));
//  	AlwaysSampler sampler;
// 
// 
// }
