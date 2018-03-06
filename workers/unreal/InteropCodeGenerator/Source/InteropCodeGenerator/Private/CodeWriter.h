// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

class FCodeWriter
{
public:
	FCodeWriter();

	template <typename... T>
	FCodeWriter& Printf(const FString& Format, const T&... Args)
	{
		return Print(FString::Printf(*Format, Args...));
	}

	FCodeWriter& Print();
	FCodeWriter& Print(const FString& String);
	FCodeWriter& Indent();
	FCodeWriter& Outdent();
	FCodeWriter& StartScope();
	FCodeWriter& EndScope();

	void WriteToFile(const FString& Filename);
	void Dump();

private:
	FString OutputSource;
	int Scope;
};
