#pragma once

#include <deque>

class LowlevelRenderer;
class HighlevelRenderer;
class FSceneNode;

class SceneManager
{
public:
  void Initialize(LowlevelRenderer* pLLRenderer, HighlevelRenderer* pHLRenderer);
  void Shutdown();

  void Validate();

  void PushScene(FSceneNode* pFrame);
  void PopScene(FSceneNode* pFrame);
private:
  LowlevelRenderer* m_LLRenderer = nullptr;
  HighlevelRenderer* m_HLRenderer = nullptr;

  std::deque<std::pair<bool/*processed*/, FSceneNode*>> m_SceneStack;
} extern g_SceneManager;
