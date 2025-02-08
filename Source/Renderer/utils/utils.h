#pragma once

class FrameContextManager
{
public:
  struct Context
  {
    FSceneNode* frameSceneNode = nullptr;

    std::shared_ptr<FSceneNode> skyframeSceneNode;
    bool frameIsRasterized = false;
    bool frameIsSkybox = false;
    bool renderingUI = false;

    struct DrawCall
    {
      AActor* Owner = nullptr;
      AActor* LightSink = nullptr;
      FSpanBuffer* SpanBuffer = nullptr;
      AZoneInfo* Zone = nullptr;
      FCoords Coords;
      std::optional<FCoords> SpecialCoords;
      FVolActorLink* LeafLights = nullptr;
      FActorLink* Volumetrics = nullptr;
      DWORD PolyFlags = 0;
      UTexture* LastTextureInfo = nullptr;
      bool InViewSpace = false;
      std::optional<TextureMetaData> LastTextureMetadata;
      std::optional<D3DXMATRIX> worldMatrix;
      std::optional<D3DXMATRIX> worldMatrixInv;
      DrawCall() = default;
    };
    std::optional<DrawCall> drawcallInfo;

    struct
    {
      std::optional<float> maxOccludeBspDistance;
      bool skipDynamicFiltering = false;
      bool bypassSpanBufferRasterization = false;
      bool levelChanged = false;
      bool bypassSetupDynamics = false;
      bool disableFDynamicSpriteSetup = false;
      bool enableViewportXYOffsetWorkaround = false;
    } overrides;
  };
  struct ScopedContext
  {
    ScopedContext() { g_ContextManager.PushFrameContext(); }
    ~ScopedContext() { g_ContextManager.PopFrameContext(); }
    Context* operator->() { return g_ContextManager.GetContext(); }
  };
private:
  static std::deque<Context> m_stack;
public:
  Context* GetContext();
  void PushFrameContext();
  void PopFrameContext();
protected:
} extern g_ContextManager;

///

static D3DXVECTOR3 UEVecToD3DXVec3(const FVector& pV)
{
  return D3DXVECTOR3{pV.X, pV.Y, pV.Z};
}

static FVector D3DXVec3ToUEVec(const D3DXVECTOR3& pV)
{
  return FVector{pV.x, pV.y, pV.z};
}

static D3DVECTOR UEVecToD3DVec3(const FVector& pV)
{
  return D3DVECTOR{pV.X, pV.Y, pV.Z};
}

static FVector D3DVec3ToUEVec(const D3DVECTOR& pV)
{
  return FVector{pV.x, pV.y, pV.z};
}

static D3DVECTOR D3DVec3ToD3DXVec3(const D3DVECTOR& pV)
{
  return D3DVECTOR{pV.x, pV.y, pV.z};
}

static D3DVECTOR D3DXVec3ToD3DVec(const D3DXVECTOR3& pV)
{
  return D3DVECTOR{pV.x, pV.y, pV.z};
}

static D3DXQUATERNION UERotatorToD3DQuaternion(const FRotator& pV)
{
  D3DXQUATERNION q{};
  D3DXQuaternionRotationYawPitchRoll(&q, (float(pV.Yaw)/65535.0f)*(PI*2.0f), (float(pV.Pitch)/65535.0f)*(PI*2.0f), (float(pV.Roll)/65535.0f)*(PI*2.0f));
  return q;
}

static D3DXMATRIX UERotatorToMatrix(const FRotator& pV)
{
  D3DXMATRIX m;
  D3DXMatrixIdentity(&m);

  auto space = FCoords() * pV;

  FVector axis[] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };
  axis[0].TransformVectorBy(space);
  axis[1].TransformVectorBy(space);
  axis[2].TransformVectorBy(space);

  m(0, 0) = axis[0].X;
  m(0, 1) = axis[0].Y;
  m(0, 2) = axis[0].Z;

  m(1, 0) = axis[1].X;
  m(1, 1) = axis[1].Y;
  m(1, 2) = axis[1].Z;

  m(2, 0) = axis[2].X;
  m(2, 1) = axis[2].Y;
  m(2, 2) = axis[2].Z;

  return m;
}

struct Stats
{
  void OnTick();

  void BeginFrame();
  void EndFrame();

  void BeginScene();
  void EndScene();

  void DrawCall();
  void DrawSkyBox();
  void DrawMainFrame();

  Stats& Writer() const;

  int32_t sceneID = -1;

  uint32_t skyBoxCount = 0;
  uint32_t skyBoxCountTotal = 0;
  uint32_t frameCount = 0;
  uint32_t sceneCount = 0;
  uint32_t sceneCountTotal = 0;
  uint32_t drawCallCount = 0;
  uint32_t drawCallCountTotal = 0;
  uint32_t sceneDrawCallCount[8]{ 0 };
  uint32_t mainFrameCount = 0;
  uint32_t mainFrameCountTotal = 0;

} extern g_Stats;