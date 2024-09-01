#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <bitset>
#include "utils/utils.h"
#include "hacks/misc.h"

#include "rendering/dxtexturemanager.h"

template <typename T>
struct Rect
{
	T Left{};
	T Top{};
	T Width{};
	T Height{};

	T Right () { return (Width + Left); }
	T Bottom() { return (Height + Top) ; }

	Rect() = default;
	Rect(const T& pLeft, const T& pTop, const T& pWidth, const T& pHeight) :
		Left(pLeft), Top(pTop), Width(pWidth), Height(pHeight) {};
};
using FloatRect = Rect<float>;
using IntRect = Rect<int32_t>;
using UIntRect = Rect<uint32_t>;

struct RangeDefinition
{
	float NearRange;
	float FarRange;
	float DepthMin;
	float DepthMax;
};

namespace RenderRanges
{
	static const RangeDefinition Engine{
		/*NearRange = */0.0f,
		/*FarRange =  */32768.0f,
		/*DepthMin =  */0.001f,
		/*DepthMax =  */1.0f,
	};

	static const RangeDefinition Game{
		/*NearRange = */0.500f,
		/*FarRange =  */Engine.FarRange - 100.0f,
		/*DepthMin =  */0.05f,
		/*DepthMax =  */Engine.DepthMax - 0.01f,
	};

	static const RangeDefinition UI{
		/*NearRange = */0.0f,
		/*FarRange =  */0.500f,
		/*DepthMin =  */Engine.DepthMin,
		/*DepthMax =  */Game.DepthMin,
	};

	static const RangeDefinition Skybox{
		/*NearRange = */Game.FarRange,
		/*FarRange =  */Engine.FarRange,
		/*DepthMin =  */Game.DepthMax,
		/*DepthMax =  */Engine.DepthMax,
	};

	extern const RangeDefinition& FromContext(FrameContextManager::Context* pCtx);
}

class LowlevelRenderer
{
	friend class HighlevelRenderer;
	friend class MaterialDebugger;
public:
	struct VertexPos3Tex0;
	struct VertexPos3Tex0Tex1;
	struct VertexPos3Norm3Tex0;
	struct VertexPos3Tex0to4;
	struct VertexPos4Color0Tex0;
	struct VertexPos3Color0;
public:
	bool Initialize(HWND hWnd, uint32_t pWidth, uint32_t pHeight, uint32_t pColorBytes, bool pFullscreen);
	void Shutdown();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// State management
	void BeginScene();
	void EndScene();
	void BeginFrame();
	void EndFrame();
  void OnLevelChange() {};
	
	void CheckDirtyMatrices();
	void SetWorldMatrix(const D3DMATRIX& pMatrix);
	void SetViewMatrix(const D3DMATRIX& pMatrix);
	void SetProjectionMatrix(const D3DMATRIX& pMatrix);

	std::pair<uint32_t,uint32_t> GetDisplaySurfaceSize() const { return {m_outputSurface.width, m_outputSurface.height}; }
	bool ResizeDisplaySurface(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight, bool pFullscreen);
	bool SetViewport(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight);
	void GetViewport(uint32_t& pmLeft, uint32_t& pmTop, uint32_t& pmWidth, uint32_t& pmHeight);
	void GetClipRects(UIntRect& pmLeft, UIntRect& pmTop, UIntRect& pmRight, UIntRect& pmBottom);

	void SetViewportDepth(const RangeDefinition& pType);
	void SetViewportDepth(float pMinZ, float pMaxZ);
	void ResetViewportDepth();
	bool ValidateViewport();
	void InitializeDeviceState();
	void PushDeviceState();
	void PopDeviceState();
	DWORD GetRenderState(D3DRENDERSTATETYPE State);
	HRESULT SetRenderState(D3DRENDERSTATETYPE State,DWORD Value);
	HRESULT SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
	HRESULT SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	void ConfigureBlendState(UnrealBlendFlags pFlags);
	void ConfigureTextureStageState(int pStageID, UnrealPolyFlags pFlags);
	void ConfigureSamplerState(int pStageID, UnrealPolyFlags pFlags);
	void SetProjectionState();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::vector<D3DDISPLAYMODE> GetDisplayModes() const;
  std::optional<D3DDISPLAYMODE> FindClosestResolution(uint32_t pWidth, uint32_t pHeight) const;
	void ClearDepth();
	void ClearDisplaySurface(const Vec4& clearColor);
	void RenderTriangleListBuffer(DWORD pFVF, const void* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pVertexSize, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Tex0to4* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Norm3Tex0* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Tex0* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Tex0Tex1* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos4Color0Tex0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Color0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void EmitDebugText(const wchar_t* pTxt);
	void DisableLight(int32_t index);
	void RenderLight(int32_t index, const D3DLIGHT9& pLight);
	void FlushLights();

	bool AllocateTexture(DeusExD3D9TextureHandle& pmTexture);
	bool SetTextureOnDevice(const uint32_t pSlot, const DeusExD3D9Texture* pTexture);

protected:

	LPDIRECT3DDEVICE9 getDevice() { return m_Device; }
private:
	struct State
	{
		static constexpr uint32_t MAX_RENDERSTATES = 512;
		static constexpr uint32_t MAX_TEXTURESLOTS = 8;
		static constexpr uint32_t MAX_TEXTURESTAGES = 8;
		static constexpr uint32_t MAX_TEXTURESTAGESTATES = 32;
		static constexpr uint32_t MAX_SAMPLERSTATES = 16;
		std::optional<IDirect3DTexture9*> m_TextureSlots[MAX_TEXTURESLOTS];
		std::optional<DWORD> m_RenderStates[MAX_RENDERSTATES];
		std::optional<DWORD> m_TextureStageStates[MAX_TEXTURESTAGES][MAX_TEXTURESTAGESTATES];
		std::optional<DWORD> m_SamplerStates[MAX_TEXTURESTAGES][MAX_SAMPLERSTATES];
		std::optional<uint32_t> m_ViewportLeft;
		std::optional<uint32_t> m_ViewportTop;
		std::optional<uint32_t> m_ViewportWidth;
		std::optional<uint32_t> m_ViewportHeight;
		std::optional<float> m_ViewportMinZ;
		std::optional<float> m_ViewportMaxZ;
		std::optional<D3DMATRIX> m_WorldMatrix;
		std::optional<D3DMATRIX> m_WorldMatrixPending;
		std::optional<D3DMATRIX> m_ViewMatrix;
		std::optional<D3DMATRIX> m_ViewMatrixPending;
		std::optional<D3DMATRIX> m_ProjectionMatrix;
		std::optional<D3DMATRIX> m_ProjectionMatrixPending;
	} m_States[16];
	State* m_CurrentState = &m_States[0];

	std::optional<uint32_t> m_DesiredViewportLeft;
	std::optional<uint32_t> m_DesiredViewportTop;
	std::optional<uint32_t> m_DesiredViewportWidth;
	std::optional<uint32_t> m_DesiredViewportHeight;
	std::optional<float> m_DesiredViewportMinZ;
	std::optional<float> m_DesiredViewportMaxZ;

	bool m_CanFlushLights = false;
private:
	using ResourceMap = std::unordered_multimap<uint32_t, void*>;

#pragma pack(push, 1)
	struct VertexPos3Tex0
	{
		D3DXVECTOR3 Pos;
		D3DXVECTOR2 Tex0;
	};

	struct VertexPos3Tex0Tex1
	{
		D3DXVECTOR3 Pos;
		D3DXVECTOR2 Tex0;
		D3DXVECTOR2 Tex1;
	};

	struct VertexPos3Norm3Tex0
	{
		D3DXVECTOR3 Pos;
		D3DXVECTOR3 Normal;
		D3DXVECTOR2 Tex0;
	};

	struct VertexPos3Tex0to4
	{
		D3DXVECTOR3 Pos;
		//uint32_t Color=0xFFFFFFFF;
		D3DXVECTOR2 Tex0;
		D3DXVECTOR2 Tex1;
		D3DXVECTOR2 Tex2;
		D3DXVECTOR2 Tex3;
		D3DXVECTOR2 Tex4;
	};

	struct VertexPos4Color0Tex0
	{
		D3DXVECTOR4 Pos;
		uint32_t Color=0xFFFFFFFF;
		D3DXVECTOR2 Tex0;
	};

	struct VertexPos3Color0
	{
		D3DXVECTOR3 Pos;
		uint32_t Color=0xFFFFFFFF;
	};
#pragma pack(pop, 1)

	struct BufferedGeoValue
	{
		IDirect3DVertexBuffer9* buffer = nullptr;
		ResourceMap* resourceAgeTracker = nullptr;
	};

	LPDIRECT3D9 m_API = nullptr;
	LPDIRECT3DDEVICE9 m_Device = nullptr;
	IDirect3DSurface9* m_DepthStencilSurface = nullptr;
	
	ResourceMap m_OldResources[10];
	ResourceMap* m_FrameOldResources = &m_OldResources[0];
	uint8_t m_FrameOldResourcesIdx = 0;
	UnrealBlendFlags m_currentBlendStateFlags = 0;
	std::unordered_map<uint32_t, BufferedGeoValue> m_bufferedGeo;
	int64_t m_vtxBufferAllocations = 0;
	IDirect3DVertexBuffer9* m_fakeLightBuffer = nullptr;
	bool m_IsInFrame = false;
	int32_t m_IsInScene = 0;
	struct
	{
		uint32_t width = 0;
		uint32_t height = 0;
		bool fullscreen = false;
		//HWND hwnd{};
		uint32_t colorBytes = 0;
	} m_outputSurface;

	D3DCAPS9 m_caps = {};
};