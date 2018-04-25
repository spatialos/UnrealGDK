// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestEmptyData.h"

UTestEmptyData::UTestEmptyData()
{
	Underlying.Reset(new test::TestEmptyData());
}

UTestEmptyData* UTestEmptyData::Init(const test::TestEmptyData& underlying)
{
    Underlying.Reset(new test::TestEmptyData(underlying));
	return this;
}


test::TestEmptyData UTestEmptyData::GetUnderlying()
{
	return *Underlying.Get();
}
