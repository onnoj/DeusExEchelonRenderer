#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "hacks/misc.h"

#include "rendering/llrenderer.h"
#include "rendering/dxtexturemanager.h"
#include "rendering/lightmanager.h"
#include "utils/materialdebugger.h"

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
	void SetViewState(FSceneNode* Frame, ViewType viewType);
	void SetProjectionState(FSceneNode* Frame, ProjectionType projection);

	void Draw2DScreenQuad(FSceneNode* Frame, float pX, float pY, float pWidth, float pHeight, uint32_t pARGB = 0xFF000000ul);
	void Draw3DCube(FSceneNode* Frame, const FVector& Position, DWORD pPrimitiveFlags, const DeusExD3D9TextureHandle& pTexture, float Size = 1.0f);
	void Draw3DLine(FSceneNode* Frame, const FVector& PositionFrom, const FVector& PositionTo, FColor Color, float Size = 1.0f);
	void DrawFullscreenQuad(FSceneNode* Frame, const DeusExD3D9TextureHandle& pTexture);
	void OnRenderingBegin(FSceneNode* Frame);
	void OnRenderingEnd(FSceneNode* Frame);
	void OnSceneBegin(FSceneNode* Frame);
	void OnSceneEnd(FSceneNode* Frame);
	void OnDrawGeometryBegin(FSceneNode* Frame);
	void OnDrawGeometry(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet);
	void OnDrawGeometryEnd(FSceneNode* Frame);
	void OnDrawMeshBegin(FSceneNode* Frame, AActor* Owner);
	void OnDrawMeshPolygon(FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span);
	void OnDrawMeshEnd(FSceneNode* Frame, AActor* Owner);
	void OnDrawSprite(FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags);;
	void OnDrawUIBegin(FSceneNode* Frame);
	void OnDrawUI(FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZ, FPlane pColor, FPlane pFog, DWORD pPolyFlags);
	void OnDrawUIEnd(FSceneNode* Frame);
	void GetViewMatrix(const FCoords& FrameCoords, D3DXMATRIX& viewMatrix);
	void GetPerspectiveProjectionMatrix(FSceneNode* Frame, D3DXMATRIX& projMatrix);

	using RenderObjectStack = std::deque<std::pair<uint32_t, const void*>>;
	const RenderObjectStack& GetRenderObjectStack() const { return m_RenderObjectStack; };
	RenderObjectStack::const_reference GetRenderObjectTop() const { return m_RenderObjectStack.back(); }
	void PushRenderObject(const void* pData, uint32_t pSize);
	void PopRenderObject(uint32_t pSize);

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
	using UIMeshesVertexBuffer = std::vector<LowlevelRenderer::PreTransformedVertexPos4Color0Tex0>;
	using SpriteMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos4Color0Tex0>;
	using DynamicMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos3Norm3Tex0>;
	using DebugMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos3Tex0>;
	using GeometryMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos3Tex0Tex1>;

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
		std::unique_ptr<DynamicMeshesVertexBuffer> buffer;
		uint32_t primitiveCount = 0;
		float lastVertexSum=0.0f;
		uint32_t lastDrawcallHash = 0;
	};

	struct UIMeshesValue {
		FTextureInfo textureInfo{};
		TextureHash textureKey{};
		UnrealPolyFlags flags = 0;
		std::unique_ptr<FSceneNode> sceneNode;
		std::unique_ptr<UIMeshesVertexBuffer> buffer;
		uint32_t primitiveCount = 0;
	};

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
		std::vector<LowlevelRenderer::VertexPos3Color0> buffer;
		uint32_t primitiveCount = 0;
	};
private:
	LowlevelRenderer* m_LLRenderer = nullptr;
	LightManager m_LightManager;
	TextureManager m_TextureManager;
	MaterialDebugger m_MaterialDebugger;
	std::vector<UIMeshesValue> m_UIMeshes;
	std::vector<SpriteMeshesValue> m_SpriteMeshes;
	DebugMeshValue m_DebugMesh;
	GeometryMeshesMap m_staticGeometryMeshes;
	GeometryMeshesMap m_dynamicGeometryMeshes;
	std::unordered_multimap<DynamicMeshesKey, DynamicMeshesValue> m_dynamicMeshes;
	std::unordered_set<uint32_t> m_DrawnNodes[FBspNode::MAX_ZONES];
	std::unique_ptr<FrameContextManager::ScopedContext> m_renderingScope;
	std::deque<std::pair<uint32_t, const void*>> m_RenderObjectStack;
};