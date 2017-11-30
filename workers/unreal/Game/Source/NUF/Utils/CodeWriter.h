#pragma once


class FCodeWriter
{
public:
	FCodeWriter();
	FCodeWriter& Print();
	FCodeWriter& Print(const FString& String);

	void WriteToFile(const FString& Filename);
	void Dump();
	FCodeWriter& Indent();
	FCodeWriter& Outdent();

private:
	FString OutputSource;
	int Scope;
};
