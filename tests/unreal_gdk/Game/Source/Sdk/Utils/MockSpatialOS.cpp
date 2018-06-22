#include "MockSpatialOS.h"

#include "SpatialOSCommon.h"
#include "SpatialOSMockViewTypes.h"
#include "SpatialOSMockWorkerTypes.h"
#include "worker.h"

namespace MockSpatialOS
{
TSharedPtr<MockConnection> CreateMockConnection()
{
  return MakeShareable(new MockConnection());
}

TSharedPtr<MockView> CreateMockView()
{
  return MakeShareable(new MockView(improbable::unreal::Components{}));
}
}
