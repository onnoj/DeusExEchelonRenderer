#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "rendering/hlrenderer.h"
#include "scenemanager.h"

SceneManager g_SceneManager;


void SceneManager::Initialize(LowlevelRenderer* pLLRenderer, HighlevelRenderer* pHLRenderer)
{
  m_LLRenderer = pLLRenderer;
  m_HLRenderer = pHLRenderer;
}

void SceneManager::Shutdown()
{
  m_LLRenderer = nullptr;
  m_HLRenderer = nullptr;
}

void SceneManager::Validate()
{
  if (!m_SceneStack.empty())
  {
    auto& item = m_SceneStack.back();
    if (!item.first)
    {
      item.first = true;
      m_HLRenderer->OnSceneBegin(item.second);
    }
  }
}

void SceneManager::PushScene(FSceneNode* pFrame)
{
  m_SceneStack.push_back(std::make_pair(false,pFrame));
}

void SceneManager::PopScene(FSceneNode* pFrame)
{ 
  check(!m_SceneStack.empty());

  auto& item = m_SceneStack.back();
  if (item.first)
  {
    check(item.second == pFrame);
    m_HLRenderer->OnSceneEnd(item.second);
    item.first = false;
  }
  m_SceneStack.pop_back();
}