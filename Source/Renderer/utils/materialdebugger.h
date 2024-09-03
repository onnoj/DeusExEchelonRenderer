#pragma once

class MaterialDebugger
{
public:
  void Update(FSceneNode* Frame);
private:
  void renderAllTextures();
  void nfDecoderUtil();
  void pfDecoderUtil();
  void exportHashMappings(bool pLoadAllTextures);
};

class RenderStateDebugger
{
public:
  static void Process(LowlevelRenderer* pLLRenderer, uint32_t pDebugID);
};

class TextureStageStateDebugger
{
public:
  static void Process(LowlevelRenderer* pLLRenderer, uint32_t pDebugID);
};


extern MaterialDebugger g_MaterialDebugger;