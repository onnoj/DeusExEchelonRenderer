#pragma once

#pragma pack(push, 1)
struct VertexPos3Tex0
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex0;
	static constexpr uint32_t /*VertexPos3Tex0*/ GetFVF() { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
};

struct VertexPos3Tex0Tex1
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex0;
	D3DXVECTOR2 Tex1;
	static constexpr uint32_t /*VertexPos3Tex0Tex1*/ GetFVF() { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE | D3DFVF_TEX1 | */ D3DFVF_TEX2 /*| D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5 */ | D3DFVF_TEXCOORDSIZE2(2) | D3DFVF_TEXCOORDSIZE2(1); }
};

struct VertexPos3Norm3Tex0
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 Tex0;
	static constexpr uint32_t /*VertexPos3Norm3Tex0*/ GetFVF() { return D3DFVF_XYZ | D3DFVF_NORMAL | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
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
	static constexpr uint32_t /*VertexPos3Tex0to4*/ GetFVF() { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
};

struct VertexPos4Color0Tex0
{
	D3DXVECTOR4 Pos;
	uint32_t Color=0xFFFFFFFF;
	D3DXVECTOR2 Tex0;
	static constexpr uint32_t /*VertexPos4Color0Tex0*/ GetFVF() { return D3DFVF_XYZW | D3DFVF_DIFFUSE | D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
};

struct PreTransformedVertexPos4Color0Tex0
{
	D3DXVECTOR4 Pos;
	uint32_t Color=0xFFFFFFFF;
	D3DXVECTOR2 Tex0;
	static constexpr uint32_t /*PreTransformedVertexPos4Color0Tex0*/ GetFVF() { return D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
};

struct VertexPos3Color0
{
	D3DXVECTOR3 Pos;
	uint32_t Color=0xFFFFFFFF;
	static constexpr uint32_t /*VertexPos3Color0*/ GetFVF() { return D3DFVF_XYZ | D3DFVF_DIFFUSE /*| D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/; }
};
#pragma pack(pop, 1)