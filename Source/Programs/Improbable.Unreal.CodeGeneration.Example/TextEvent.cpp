// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TextEvent.h"

UTextEvent::UTextEvent()
{
	Underlying.Reset(new test::TextEvent(""));
}

UTextEvent* UTextEvent::Init(const test::TextEvent& underlying)
{
    Underlying.Reset(new test::TextEvent(underlying));
	return this;
}

FString UTextEvent::GetText()
{
    return FString(Underlying->text().c_str());
}
UTextEvent* UTextEvent::SetText(FString text)
{
    Underlying->set_text(TCHAR_TO_UTF8(*text));
	return this;
}


test::TextEvent UTextEvent::GetUnderlying()
{
	return *Underlying.Get();
}
