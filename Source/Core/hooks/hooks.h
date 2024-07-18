#pragma once

namespace Hooks
{
  namespace UGameEngineCallbacks
  {
    using NotifyLevelChangeMap = std::unordered_multimap<void*, std::function<void()>>;
    extern NotifyLevelChangeMap OnNotifyLevelChange;

    using TickMap = std::unordered_multimap<void*, std::function<void(FLOAT DeltaSeconds)>>;
    extern TickMap OnTick;
  }
}

extern void InstallUGameEngineHooks();
extern void UninstallUGameEngineHooks();