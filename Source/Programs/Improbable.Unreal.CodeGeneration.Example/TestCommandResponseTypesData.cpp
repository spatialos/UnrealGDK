// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandResponseTypesData.h"

UTestCommandResponseTypesData::UTestCommandResponseTypesData()
{
	Underlying.Reset(new test::TestCommandResponseTypesData());
}

UTestCommandResponseTypesData* UTestCommandResponseTypesData::Init(const test::TestCommandResponseTypesData& underlying)
{
    Underlying.Reset(new test::TestCommandResponseTypesData(underlying));
	return this;
}


test::TestCommandResponseTypesData UTestCommandResponseTypesData::GetUnderlying()
{
	return *Underlying.Get();
}
