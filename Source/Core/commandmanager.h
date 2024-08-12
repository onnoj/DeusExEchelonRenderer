#pragma once

#include <deque>

class CommandManager
{
public:
  //using Command = std::pair<std::function<void()>, std::function<bool()>>;
  struct Command
  {
    std::function<void()> m_OnInit;
    std::function<bool()> m_IsFinished;

    Command(std::function<void()> pOnInit, std::function<bool()> pIsFinished) : m_OnInit(pOnInit), m_IsFinished(pIsFinished){};
  };
public:
  void Initialize();
  void Shutdown();

  void OnTick(FLOAT DeltaTime);
  UBOOL OnExec(const TCHAR* Cmd, FOutputDevice& Ar);

  void RegisterConsoleCommand(const wchar_t* pCommand, std::function<void()>&& pFunc);
  void UnregisterConsoleCommand(const wchar_t* pCommand);

  void QueueCommand(std::unique_ptr<Command>&& pCommand);

  template <typename TCommand, typename... TArgs>
  void QueueCommand(TArgs&&... args)
  {
    QueueCommand(std::make_unique<TCommand>(std::forward<TArgs...>(args...)));
  }
private:
  bool m_WaitForCommandToFinish = false;
  std::unordered_map<uint32_t, std::function<void()>> m_ConsoleCommands;
  std::deque<std::unique_ptr<Command>> m_CommandQueue;
} extern g_CommandManager;

class WaitTimeCommand : public CommandManager::Command
{
private:
  uint32_t m_waitMiliseconds = 0;
  uint64_t m_Now = 0;
  std::chrono::system_clock::time_point m_Deadline;
public:
  WaitTimeCommand() = delete;
  WaitTimeCommand(uint32_t pWaitMiliseconds) : m_waitMiliseconds(pWaitMiliseconds), 
    CommandManager::Command
    {
            [&](){
              m_Deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(m_waitMiliseconds);
            },
            [&](){
              return (std::chrono::system_clock::now() > m_Deadline);
            }
    }{};
};

class ChangeLevelCommand : public CommandManager::Command
{
  private:
    std::wstring m_LevelName;
  public:
    ChangeLevelCommand() = delete;
    ChangeLevelCommand(const wchar_t* pLevelName) : m_LevelName(pLevelName),
      CommandManager::Command
      {
        [&](){
          GEngine->SetClientTravel(GEngine->Client->Viewports(0), m_LevelName.c_str(), 0, ETravelType::TRAVEL_Partial);
        },
        [&](){
          return (GEngine->GPendingLevel==nullptr && GEngine->GLevel->GetLevelInfo()->LevelAction == LEVACT_None); 
        }
      }{};
};