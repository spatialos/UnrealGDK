#pragma once

#include "Runtime/Core/Public/Templates/SharedPointer.h"

class MockConnection;
class MockView;

namespace MockSpatialOS
{
TSharedPtr<MockConnection> CreateMockConnection();
TSharedPtr<MockView> CreateMockView();
}