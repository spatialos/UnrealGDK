// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestEmptyComponentUpdate.h"

test::TestEmpty::Update UTestEmptyComponentUpdate::DefaultUnderlying = test::TestEmpty::Update();

UTestEmptyComponentUpdate::UTestEmptyComponentUpdate()
{
}

UTestEmptyComponentUpdate* UTestEmptyComponentUpdate::Init(const test::TestEmpty::Update& underlying)
{
	return InitInternal(underlying);
}

UTestEmptyComponentUpdate* UTestEmptyComponentUpdate::Reset()
{
	return ResetInternal();
}

const test::TestEmpty::Update UTestEmptyComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UTestEmptyComponentUpdate* UTestEmptyComponentUpdate::InitInternal(const test::TestEmpty::Update& underlying)
{
    Underlying = test::TestEmpty::Update(underlying);
	return this;
}

UTestEmptyComponentUpdate* UTestEmptyComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

const test::TestEmpty::Update UTestEmptyComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
