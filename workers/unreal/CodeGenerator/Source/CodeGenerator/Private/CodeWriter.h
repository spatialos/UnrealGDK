// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

class FCodeWriter
{
public:
	FCodeWriter();

	FCodeWriter& Print();
	FCodeWriter& Print(const FString& String);
	FCodeWriter& Indent();
	FCodeWriter& Outdent();

	void WriteToFile(const FString& Filename);

private:
	FString OutputSource;
	int Scope;
};
