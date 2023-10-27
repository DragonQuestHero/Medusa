#pragma once
#include <ntifs.h>
#include <intrin.h>

void TestALL(PDRIVER_OBJECT drive_object);
void TestWalkStack();
void TestReadKernelMemory();
void TestGetKernel(PDRIVER_OBJECT drive_object);
void TestCopyMemory(PDRIVER_OBJECT drive_object);