#include "DeusExEchelonCore_PCH.h"
#pragma hdrstop

#include "hooks/hooks.h"
#include "coreutils.h"
#include "demomanager.h"
#include "commandmanager.h"

DemoManager g_DemoManager;

void DemoManager::Initialize()
{
  Hooks::UGameEngineCallbacks::OnTick.insert(std::make_pair(this, [&](FLOAT pTime){ OnTick(pTime); }));
  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.insert(std::make_pair(this, [&](){ OnNotifyLevelChange(); }));
  
  g_CommandManager.RegisterConsoleCommand(L"bm", [&](){
    RunBenchmark();
  });
  g_CommandManager.RegisterConsoleCommand(L"printori", [&](){
    auto viewport = GEngine->Client->Viewports(0);
    GLog->Logf(L"{L\"PicX\", "
                      "L\"%s\", " /*level name*/
                      "{%.8f, %.8f, %.8f}, " /*actor pos*/
                      "{%d, %d, %d}, " /*actor rot*/
                      "{%d, %d, %d} }," /*camera rot*/,
          GEngine->GLevel->GetOuter()->GetName(),
          viewport->Actor->Location.X, viewport->Actor->Location.Y, viewport->Actor->Location.Z,
          viewport->Actor->Rotation.Pitch, viewport->Actor->Rotation.Yaw, viewport->Actor->Rotation.Roll,
          viewport->Actor->ViewRotation.Pitch, viewport->Actor->ViewRotation.Yaw, viewport->Actor->ViewRotation.Roll
      );
  });
}

void DemoManager::Shutdown()
{
  g_CommandManager.UnregisterConsoleCommand(L"bm");
  g_CommandManager.UnregisterConsoleCommand(L"printori");
  Hooks::UGameEngineCallbacks::OnTick.erase(this);
  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.erase(this);

}

void DemoManager::OnTick(FLOAT DeltaTime)
{
}

void DemoManager::OnNotifyLevelChange()
{
}

//class __declspec(dllimport) UWindowsViewport : public UViewport
//{
//public:
//  void TryRenderDevice( const TCHAR* ClassName, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );
//};

void DemoManager::RunBenchmark()
{
  static const std::tuple<const wchar_t*, const wchar_t*, FVector, FRotator, FRotator> locations[] =
  {
    {L"Pic01", L"06_HongKong_WanChai_Underworld", {-319.66137695, -716.39184570, -336.10000610}, {0, -6194, 0}, {64442, -6194, 0}},
    {L"Pic02", L"06_HongKong_WanChai_Underworld", {-1110.02636719, -1302.69787598, -96.19999695}, {0, -16953, 0}, {65264, -16953, 0}},
    {L"Pic03", L"06_HongKong_WanChai_Underworld", {-228.73016357, -170.10530090, -272.29998779}, {0, -58722, 0}, {65516, -58722, 0}},
    {L"Pic04", L"06_HongKong_WanChai_Market", {-172.62487793, -399.45581055, 47.70001221}, {0, -2314, 0}, {65100, -2314, 0}},
    {L"Pic05", L"01_NYC_UNATCOHQ", {-2055.61840820, 825.85650635, 607.90002441}, {0, 60684, 0}, {65427, 60684, 0} },
    {L"Pic06", L"01_NYC_UNATCOIsland", {-4476.43359375, 10456.89453125, -256.25000000}, {0, -6262, 0}, {333, -6262, 0} },
    {L"Pic07", L"02_NYC_Bar", {86.26783752, 92.95317078, 47.75000000}, {0, -27932, 0}, {0, -27932, 0} },
    {L"Pic08", L"02_NYC_BatteryPark", {-3371.49975586, -3446.59814453, 438.64245605}, {0, 8467, 0}, {63570, 8467, 0} },
    {L"Pic09", L"02_NYC_Street", {2279.32861328, -325.05255127, -1118.65844727}, {0, 51521, 0}, {65231, 51521, 0} },
    {L"Pic10", L"02_NYC_Street", {2122.73803711, -13.15898991, -464.20001221}, {0, 26319, 0}, {2167, 26319, 0} },
    {L"Pic11", L"02_NYC_Street", {1239.18957520, -1340.80566406, -464.29998779}, {0, 40127, 0}, {503, 40127, 0} },
    {L"Pic12", L"02_NYC_Hotel", {-2.12176180, -808.76263428, -16.25000000}, {0, -20812, 0}, {65395, -20812, 0} },
    {L"Pic13", L"06_HongKong_WanChai_Canal", {803.39898682, 1512.93786621, 47.74999619}, {0, -29640, 0}, {65526, -29640, 0} },
    {L"Pic14", L"06_HongKong_WanChai_Market", {827.59204102, -794.83300781, 47.70000076}, {0, -30871, 0}, {65403, -30871, 0} },
    {L"Pic15", L"11_Paris_Cathedral", {-55.28193665, -886.10736084, 670.65234375}, {0, -66584, 0}, {63627, -66584, 0} },
    {L"Pic16", L"10_Paris_Metro", {420.24987793, -1079.28369141, 716.80767822}, {0, 19524, 0}, {64264, 19524, 0} },
    {L"Pic17", L"14_Vandenberg_Sub", {5709.94238281, 3516.98583984, 145.94595337}, {0, -30465, 0}, {63849, -30465, 0} },
    {L"Pic18", L"01_NYC_UNATCOHQ", {1625.67956543, -534.12329102, 27.80000114}, {0, 6055, 0}, {63062, 6055, 0} },
  };

  for (const auto& location : locations)
  {
    g_CommandManager.QueueCommand<ChangeLevelCommand>(std::get<1>(location));
    g_CommandManager.QueueCommand(std::make_unique<CommandManager::Command>(
      [=](){
        auto viewport = GEngine->Client->Viewports(0);
        GEngine->GLevel->GetLevelInfo()->bPlayersOnly = 0;
        viewport->Exec(L"showhud 0", *GLog);
        viewport->Actor->SetCollision(FALSE, FALSE, FALSE);
        viewport->Actor->bCollideWorld = false;
        viewport->Actor->UnderWaterTime = -1.0;
        auto oldAnimFrame = viewport->Actor->AnimFrame;
        auto oldAnimSeq = viewport->Actor->AnimSequence;
        viewport->Actor->bCanSwim = false;
        viewport->Actor->setPhysics(0);
      },
      [&](){ return true; }
    ));
    g_CommandManager.QueueCommand<WaitTimeCommand>(100);
    g_CommandManager.QueueCommand(std::make_unique<CommandManager::Command>(
      [=](){
        auto viewport = GEngine->Client->Viewports(0);
        GEngine->GLevel->FarMoveActor(viewport->Actor, std::get<2>(location));
        GEngine->GLevel->MoveActor(viewport->Actor, {0,0,0}, std::get<3>(location), FCheckResult{});
        viewport->Actor->ViewRotation = std::get<4>(location);
      },
      [&](){ return true; }
    ));
    g_CommandManager.QueueCommand(std::make_unique<CommandManager::Command>(
      [=](){
        GEngine->GLevel->GetLevelInfo()->bPlayersOnly = 1;
      },
      [&](){ return true; }
    ));
    g_CommandManager.QueueCommand<WaitTimeCommand>(1000);
    g_CommandManager.QueueCommand(std::make_unique<CommandManager::Command>(
      [=](){
        std::filesystem::path p = Utils::GetProcessFolder();
        p /= std::get<0>(location);
        p += ".bmp";
        auto viewport = GEngine->Client->Viewports(0);
        Utils::Screenshot((HWND)viewport->GetWindow(), (p ));
      },
      [&](){ return true; }
    ));
    g_CommandManager.QueueCommand<WaitTimeCommand>(100);
  }
}