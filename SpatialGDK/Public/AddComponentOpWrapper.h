// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "SpatialOSWorkerTypes.h"

class AddComponentOpWrapperBase
{
public:
};

<template T>
class AddComponentOpWrapper : AddComponentOpWrapperBase
{
public:
  worker::AddComponentOp<T> Op;
};