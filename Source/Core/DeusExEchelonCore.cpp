#include "DeusExEchelonCore_PCH.h"
#pragma hdrstop

void InitializeEchelonCore()
{
  TObjectIterator<UGameEngine> EngineIt;
  GEngine = *EngineIt;
}

void ShutdownEchelonCore()
{
  GEngine = nullptr;
}