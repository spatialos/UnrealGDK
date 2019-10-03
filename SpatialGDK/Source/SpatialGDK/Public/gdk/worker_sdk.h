#ifndef GDK_WORKER_SDK_H
#define GDK_WORKER_SDK_H
#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace gdk {

using EntityId = Worker_EntityId;
using ComponentId = Worker_ComponentId;
using CommandIndex = Worker_CommandIndex;
using RequestId = Worker_RequestId;
using FieldId = Schema_FieldId;

}  // namespace gdk
#endif  // GDK_WORKER_SDK_H
