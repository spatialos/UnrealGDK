// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandResponseTypesComponentUpdate.h"

test::TestCommandResponseTypes::Update UTestCommandResponseTypesComponentUpdate::DefaultUnderlying = test::TestCommandResponseTypes::Update();

UTestCommandResponseTypesComponentUpdate::UTestCommandResponseTypesComponentUpdate()
{
}

UTestCommandResponseTypesComponentUpdate* UTestCommandResponseTypesComponentUpdate::Init(const test::TestCommandResponseTypes::Update& underlying)
{
	return InitInternal(underlying);
}

UTestCommandResponseTypesComponentUpdate* UTestCommandResponseTypesComponentUpdate::Reset()
{
	return ResetInternal();
}

const test::TestCommandResponseTypes::Update UTestCommandResponseTypesComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UTestCommandResponseTypesComponentUpdate* UTestCommandResponseTypesComponentUpdate::InitInternal(const test::TestCommandResponseTypes::Update& underlying)
{
    Underlying = test::TestCommandResponseTypes::Update(underlying);
	return this;
}

UTestCommandResponseTypesComponentUpdate* UTestCommandResponseTypesComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

const test::TestCommandResponseTypes::Update UTestCommandResponseTypesComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
