#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "hacks/misc.h"

#include "rendering/llrenderer.h"
#include "rendering/dxtexturemanager.h"
#include "rendering/lightmanager.h"
#include "rendering/renderobjectmanager.h"
#include "utils/materialdebugger.h"

enum class RenderCommandQueue
{
	main,
	staticGeo,
	dynamicGeo,
	dynamicMesh,
	ui,
	pfx,
	COUNT,
};
using RenderCall = std::function<void()>;
constexpr uint32_t RenderCommandQueueMax = static_cast<uint32_t>(RenderCommandQueue::COUNT);

class HighlevelRenderer
{
public:
	enum class ViewType { identity, engine, game };
	enum class ProjectionType { identity, orthogonal, uiorthogonal, perspective };
public:
	void Initialize(LowlevelRenderer* pLLRenderer);
	void Shutdown();

	void OnLevelChange();

	void SetWorldTransformStateToIdentity();
	void SetWorldTransformState(const D3DXMATRIX& pMatrix);
	void SetViewState(const FSceneNode* Frame, ViewType viewType);
	void SetProjectionState(const FSceneNode* Frame, ProjectionType projection);

	void DrawPlayerBody(const FSceneNode* Frame);
	void Draw2DScreenQuad(const FSceneNode* Frame, float pX, float pY, float pWidth, float pHeight, uint32_t pARGB = 0xFF000000ul);
	void Draw3DCube(const FSceneNode* Frame, const FVector& Position, DWORD pPrimitiveFlags, const DeusExD3D9TextureHandle& pTexture, float Size = 1.0f);
	void Draw3DLine(const FSceneNode* Frame, const FVector& PositionFrom, const FVector& PositionTo, FColor Color, float Size = 1.0f);
	void DrawFullscreenQuad(const FSceneNode* Frame, const DeusExD3D9TextureHandle& pTexture);
	void OnRenderingBegin(const FSceneNode* Frame);
	void OnRenderingEnd(const FSceneNode* Frame);
	void OnSceneBegin(const FSceneNode* Frame);
	void OnSceneEnd(const FSceneNode* Frame);
	void OnDrawGeometryBegin(const FSceneNode* Frame);
	void OnDrawGeometry(const FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet);
	void OnDrawGeometryOld(const FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet);
	void OnDrawGeometryEnd(const FSceneNode* Frame);
	void OnDrawMeshBegin(const FSceneNode* Frame, AActor* Owner);
	void OnDrawMeshPolygon(const FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span);
	void OnDrawMeshEnd(const FSceneNode* Frame, AActor* Owner);
	void OnDrawSprite(const FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags);;
	void OnDrawUIBegin(const FSceneNode* Frame);
	void OnDrawUI(const FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags);
	void OnDrawUIEnd(const FSceneNode* Frame);
	void GetViewMatrix(const FCoords& FrameCoords, D3DXMATRIX& viewMatrix);
	void GetPerspectiveProjectionMatrix(const FSceneNode* Frame, D3DXMATRIX& projMatrix);


	using RenderObjectStack = std::deque<std::pair<uint32_t, const void*>>;
	const RenderObjectStack& GetRenderObjectStack() const { return m_RenderObjectStack; };
	RenderObjectStack::const_reference GetRenderObjectTop() const { return m_RenderObjectStack.back(); }
	void PushUERenderObject(const void* pData, uint32_t pSize);
	void PopUERenderObject(uint32_t pSize);

	void AddRenderCommand(RenderCommandQueue pQueue, std::function<void()>&& pCB);
	void ExecuteCommandQueue(RenderCommandQueue pQueue);
	void ClearCommandQueue(RenderCommandQueue pQueue);

	template <typename T>
	const T* GetRenderObjectTopT() const
	{
		RenderObjectStack::const_reference ref = GetRenderObjectTop();
		check(ref.first == sizeof(T));
		if (ref.first == sizeof(T))
		{
			return reinterpret_cast<const T*>(ref.second);
		}
		else
		{
			return nullptr;
		}
	}

	TextureManager& GetTextureManager() { return m_TextureManager; }
private:
	using DynamicMeshesKey = uint32_t;
	using GeometryMeshesKey = uint32_t;
	//using UIMeshesVertexBuffer = std::vector<PreTransformedVertexPos4Color0Tex0>;
	using SpriteMeshesVertexBuffer = std::vector<VertexPos4Color0Tex0>;
	using DynamicMeshesVertexBuffer = std::vector<VertexPos3Norm3Tex0>;
	using DebugMeshesVertexBuffer = std::vector<VertexPos3Tex0>;
	using GeometryMeshesVertexBuffer = std::vector<VertexPos3Tex0Tex1>;

	struct GeometryMeshesValue {
		TextureSet textureSet{};
		DeusExD3D9TextureHandle albedoTextureHandle;
		DeusExD3D9TextureHandle lightmapTextureHandle;
		UnrealPolyFlags flags = 0;
		std::unique_ptr<GeometryMeshesVertexBuffer> buffer;
		uint32_t hash = 0;
		uint32_t primitiveCount = 0;
		uint32_t debug = 0;
		//std::optional<int32_t> zoneIndex;
		std::bitset<FBspNode::MAX_ZONES> zoneIndices;
		D3DXMATRIX worldMatrix;
		D3DXMATRIX worldMatrixInverse;
	};
	using GeometryMeshesMap = std::unordered_multimap<GeometryMeshesKey, GeometryMeshesValue>;


	struct DynamicMeshesValue {
		FTextureInfo textureInfo{};
		UnrealPolyFlags flags = 0;
		std::shared_ptr<RenderObject> renderObject;
		float lastVertexSum=0.0f;
		uint32_t lastDrawcallHash = 0;
	};
#if 0
	struct UIMeshesValue {
		FTextureInfo textureInfo{};
		TextureHash textureKey{};
		UnrealPolyFlags flags = 0;
		std::unique_ptr<FSceneNode> sceneNode;
		std::unique_ptr<UIMeshesVertexBuffer> buffer;
		uint32_t primitiveCount = 0;
	};
#endif

	struct SpriteMeshesValue {
		FTextureInfo textureInfo{};
		TextureHash textureKey{};
		D3DXMATRIX worldmatrix;
		UnrealPolyFlags flags = 0;
		std::unique_ptr<FSceneNode> sceneNode;
		std::unique_ptr<SpriteMeshesVertexBuffer> buffer;
		uint32_t primitiveCount = 0;
	};

	struct DebugMeshValue {
		UnrealPolyFlags flags = 0;
		std::vector<VertexPos3Color0> buffer;
		uint32_t primitiveCount = 0;
	};
private:
	RenderObjectManager m_RenderObjectManager;
	LowlevelRenderer* m_LLRenderer = nullptr;
	LightManager m_LightManager;
	TextureManager m_TextureManager;
	MaterialDebugger m_MaterialDebugger;
	DebugMeshValue m_DebugMesh;
	GeometryMeshesMap m_staticGeometryMeshes;
	GeometryMeshesMap m_dynamicGeometryMeshes;
	std::unordered_multimap<DynamicMeshesKey, DynamicMeshesValue> m_dynamicMeshes;
	std::unordered_set<uint32_t> m_DrawnNodes[FBspNode::MAX_ZONES];
	std::unique_ptr<FrameContextManager::ScopedContext> m_renderingScope;
	std::deque<std::pair<uint32_t, const void*>> m_RenderObjectStack;
	std::vector<RenderCall> m_CommandQueues[RenderCommandQueueMax];
};