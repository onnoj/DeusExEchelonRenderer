#include "DeusExEchelonCore_PCH.h"
#pragma hdrstop

#include "hooks/hooks.h"
#include "coreutils.h"
#include "commandmanager.h"

CommandManager g_CommandManager;
//////////////////////////////////////////////////////////////////////////////
namespace
{
  class LocalHook : public FExec
  {
    public:
      virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar ) 
      {
        if (g_CommandManager.OnExec(Cmd, Ar) != 0)
        {
          return 1;
        }

        if (m_ParentExecHook == nullptr)
        {
          return 0;
        }
        return m_ParentExecHook->Exec(Cmd, Ar);
      };
    public:
      void Connect()
      {
        m_ParentExecHook = GExec;
        GExec = &g_LocalHook;
      }
      void Disconnect()
      {
        GExec = m_ParentExecHook;
        m_ParentExecHook = nullptr;
      }
    private:
      FExec* m_ParentExecHook = nullptr;
  } static g_LocalHook;
}
//////////////////////////////////////////////////////////////////////////////
void CommandManager::OnTick(FLOAT DeltaTime)
{
  if (!m_CommandQueue.empty())
  {
    auto& cmd = m_CommandQueue.front();

    if (m_WaitForCommandToFinish)
    {
      const bool finished = cmd->m_IsFinished();
      if (finished)
      {
        m_WaitForCommandToFinish = false;
        m_CommandQueue.pop_front();
        return;
      }
    }
    else
    {
      //Trigger init func
      cmd->m_OnInit();
      
      m_WaitForCommandToFinish = true;
    }
  }
}

UBOOL CommandManager::OnExec(const TCHAR* Cmd, FOutputDevice& Ar)
{
  const auto commandKey = Utils::HashWc(Cmd);
  if (auto it = m_ConsoleCommands.find(commandKey); it != m_ConsoleCommands.end())
  {
    (*it).second();
    return 1;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////////
void CommandManager::Initialize()
{
  g_LocalHook.Connect();
  Hooks::UGameEngineCallbacks::OnTick.insert({this, [&](FLOAT pTime){ OnTick(pTime); }});
}

void CommandManager::Shutdown()
{
  Hooks::UGameEngineCallbacks::OnTick.erase(this);
  g_LocalHook.Disconnect();
}

void CommandManager::RegisterConsoleCommand(const wchar_t* pCommand, std::function<void()>&& pFunc)
{
  m_ConsoleCommands.insert(std::make_pair(Utils::HashWc(pCommand), std::move(pFunc)));
}

void CommandManager::UnregisterConsoleCommand(const wchar_t* pCommand)
{
  m_ConsoleCommands.erase(Utils::HashWc(pCommand));
}

void CommandManager::QueueCommand(std::unique_ptr<Command>&& pCommand)
{
  m_CommandQueue.push_back(std::move(pCommand));
}