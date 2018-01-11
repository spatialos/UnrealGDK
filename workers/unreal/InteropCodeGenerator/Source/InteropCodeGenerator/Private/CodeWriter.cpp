// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CodeWriter.h"
#include "Misc/FileHelper.h"

FCodeWriter::FCodeWriter() : Scope(0)
{
}

FCodeWriter& FCodeWriter::Print()
{
	OutputSource += TEXT("\n");
	return *this;
}

FCodeWriter& FCodeWriter::Print(const FString& String)
{
	TArray<FString> Lines;
	String.ParseIntoArray(Lines, TEXT("\n"), false);
	for (auto& Line : Lines)
	{
		FString ScopeIdent;
		for (int ScopeLevel = 0; ScopeLevel < Scope; ++ScopeLevel)
		{
			ScopeIdent += FString(TEXT("\t"));
		}
		OutputSource += ScopeIdent + Line.Trim() + TEXT("\n");
	}
	return *this;
}

FCodeWriter& FCodeWriter::Indent()
{
	Scope++;
	return *this;
}

FCodeWriter& FCodeWriter::Outdent()
{
	check(Scope > 0);
	Scope--;
	return *this;
}

void FCodeWriter::WriteToFile(const FString& Filename)
{
	check(Scope == 0);
	FFileHelper::SaveStringToFile(OutputSource, *Filename);
}
