#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <bitset>
#include "hacks/misc.h"

#include "rendering/dxtexturemanager.h"

class LowlevelRenderer
{
	friend class HighlevelRenderer;
	friend class MaterialDebugger;
public:
	struct VertexPos3Tex0;
	struct VertexPos3Norm3Tex0;
	struct VertexPos3Tex0to4;
	struct VertexPos4Color0Tex0;
	struct VertexPos3Color0;
	static constexpr float NearRange = 0.501f;
	static constexpr float FarRange = 32768.0f;
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
	
	void SetWorldMatrix(const D3DMATRIX& pMatrix);
	void SetViewMatrix(const D3DMATRIX& pMatrix);
	void SetProjectionMatrix(const D3DMATRIX& pMatrix);

	bool ResizeDisplaySurface(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight, bool pFullscreen);
	bool SetViewport(uint32_t pLeft, uint32_t pTop, uint32_t pWidth, uint32_t pHeight);
	void SetViewportDepth(float pMinZ, float pMaxZ);
	void ResetViewportDepth();
	bool ValidateViewport();
	void PushDeviceState();
	void PopDeviceState();
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
	void ClearDepth() {/*TODO?*/ };
	void ClearDisplaySurface(const Vec4& clearColor);
	void RenderTriangleListBuffer(DWORD pFVF, const void* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pVertexSize, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Tex0to4* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Norm3Tex0* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos3Tex0* pVertices, const uint32_t pPrimitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void RenderTriangleList(const VertexPos4Color0Tex0* pVertices, const uint32_t primitiveCount, const uint32_t pVertexCount, const uint32_t pHash, const uint32_t pDebug);
	void DisableLight(int32_t index);
	void RenderLight(int32_t index, const D3DLIGHT9& pLight);
	void FlushLights();

	bool AllocateTexture(DeusExD3D9TextureHandle& pmTexture);
	bool SetTextureOnDevice(const DeusExD3D9Texture* pTexture);

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
		std::optional<D3DMATRIX> m_ViewMatrix;
		std::optional<D3DMATRIX> m_ProjectionMatrix;
	} m_States[8];
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