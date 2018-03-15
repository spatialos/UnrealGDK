// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FFunctionSignature
{
	FString Type;
	FString NameAndParams;

	FString Declaration() const
	{
		return FString::Printf(TEXT("%s %s;"), *Type, *NameAndParams);
	}

	FString Definition() const
	{
		return FString::Printf(TEXT("%s %s"), *Type, *NameAndParams);
	}

	FString Definition(const FString& TypeName) const
	{
		return FString::Printf(TEXT("%s %s::%s"), *Type, *TypeName, *NameAndParams);
	}
};

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

	FCodeWriter& BeginScope();
	FCodeWriter& End();

	void WriteToFile(const FString& Filename);
	void Dump();

private:
	FString OutputSource;
	int Scope;
};

class FFunctionWriter
{
public:
	FFunctionWriter(FCodeWriter& Writer, const FString& Definition) : Writer(Writer)
	{
		Writer.Print(Definition);
		Writer.BeginScope();
	}

	FFunctionWriter(FCodeWriter& Writer, const FFunctionSignature& Signature) : FFunctionWriter(Writer, Signature.Definition())
	{
	}

	FFunctionWriter(FCodeWriter& Writer, const FFunctionSignature& Signature, const FString& TypeName) : FFunctionWriter(Writer, Signature.Definition(TypeName))
	{
	}

	~FFunctionWriter()
	{
		Writer.End();
	}

private:
	FCodeWriter& Writer;
};

class FScopeWriter
{
public:
	FScopeWriter(FCodeWriter& Writer) : Writer(Writer)
	{
		Writer.BeginScope();
	}

	~FScopeWriter()
	{
		Writer.End();
	}

private:
	FCodeWriter& Writer;
};