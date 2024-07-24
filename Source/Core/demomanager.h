#pragma once

class DemoManager
{
public:
  void Initialize();
  void Shutdown();

  void RunBenchmark(bool pMakeScreenshots, bool pLoop, uint32_t pDelayMs);

  void OnTick(FLOAT DeltaTime);
  void OnNotifyLevelChange();
};

extern DemoManager g_DemoManager;