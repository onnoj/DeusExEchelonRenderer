#pragma once

class DemoManager
{
public:
  void Initialize();
  void Shutdown();

  void RunBenchmark();

  void OnTick(FLOAT DeltaTime);
  void OnNotifyLevelChange();
};

extern DemoManager g_DemoManager;