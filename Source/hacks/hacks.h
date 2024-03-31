#pragma once

extern void InstallFSceneNodeHacks();
extern void InstallFSpanBufferHacks();
extern void InstallURenderHacks();
extern void InstallUGameEngineHacks();
extern void InstallFDynamicItemFilterHacks();
extern void InstallThreadAffinityHacks();
extern void InstallBytePatches();
extern void InstallRTXConfigPatches();

extern void UninstallFSceneNodeHacks();
extern void UninstallFSpanBufferHacks();
extern void UninstallURenderHacks();
extern void UninstallUGameEngineHacks();
extern void UninstallFDynamicItemFilterHacks();
extern void UninstallThreadAffinityHacks();
extern void UninstallBytePatches();
extern void UninstallRTXConfigPatches();
///

template<typename TFuncPtr>
struct HookableFunction
{
  union
  {
    TFuncPtr funcPtr;
    uint64_t func64;
  };

  HookableFunction(const TFuncPtr& pRH)
  {
    funcPtr = pRH;
    cpy.funcPtr = pRH;
  }

  HookableFunction() = delete;
  HookableFunction(const HookableFunction&) = delete;
  HookableFunction(HookableFunction&&) = delete;
  void Restore() { funcPtr = cpy.funcPtr; }
  operator TFuncPtr() { return funcPtr; }
  HookableFunction<TFuncPtr>& operator=(const TFuncPtr& pRH) { funcPtr = pRH; cpy.funcPtr = pRH; return *this; }
private:
  struct
  {
    union
    {
      TFuncPtr funcPtr;
      uint64_t func64;
    };
  } cpy;
};