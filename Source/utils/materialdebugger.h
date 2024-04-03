#pragma once

class MaterialDebugger
{
public:
  void Update(FSceneNode* Frame);
private:
  void renderAllTextures();
  void exportHashMappings(bool pLoadAllTextures);
};

extern MaterialDebugger g_MaterialDebugger;