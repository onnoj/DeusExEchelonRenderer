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

	void Draw3DCube(FSceneNode* Frame, const FVector& Position, const DeusExD3D9TextureHandle& pTexture, float Size = 1.0f);
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
	void OnDrawMesh(FSceneNode* Frame, FTextureInfo& Info, FTransTexture** Pts, int NumPts, DWORD PolyFlags, FSpanBuffer* Span);
	void OnDrawMeshEnd(FSceneNode* Frame, AActor* Owner);
	void OnDrawUIBegin(FSceneNode* Frame);
	void OnDrawUI(FSceneNode* Frame, FTextureInfo& TextureInfo, float pX, float pY, float pWidth, float pHeight, float pTexCoordU, float pTexCoordV, float pTexCoordUL, float pTexCoordVL, FSpanBuffer* Span, float pZDepth, FPlane pColor, FPlane pFog, DWORD pPolyFlags);
	void OnDrawUIEnd(FSceneNode* Frame);
	void GetViewMatrix(FSceneNode* Frame, D3DXMATRIX& viewMatrix);
	void GetPerspectiveProjectionMatrix(FSceneNode* Frame, D3DXMATRIX& projMatrix);

	TextureManager& GetTextureManager() { return m_TextureManager; }
private:
	using DynamicMeshesKey = uint32_t;
	using StaticMeshesKey = uint32_t;
	using UIMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos4Color0Tex0>;
	using DynamicMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos3Norm3Tex0>;
	using StaticMeshesVertexBuffer = std::vector<LowlevelRenderer::VertexPos3Tex0>;

	struct StaticMeshesValue {
		TextureSet textureSet{};
		DeusExD3D9TextureHandle textureHandle;
		UnrealPolyFlags flags = 0;
		std::unique_ptr<StaticMeshesVertexBuffer> buffer;
		uint32_t hash = 0;
		uint32_t primitiveCount = 0;
		uint32_t debug = 0;
		D3DXMATRIX worldMatrix;
		D3DXMATRIX worldMatrixInverse;
	};


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
	DebugMeshValue m_DebugMesh;
	std::unordered_multimap<StaticMeshesKey, StaticMeshesValue> m_staticMeshes;
	std::unordered_multimap<DynamicMeshesKey, DynamicMeshesValue> m_dynamicMeshes;
	std::unordered_set<uint32_t> m_DrawnNodes;
	std::unique_ptr<FrameContextManager::ScopedContext> m_renderingScope;
};