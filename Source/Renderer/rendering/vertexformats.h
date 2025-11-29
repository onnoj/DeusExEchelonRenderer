#pragma once

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

struct PreTransformedVertexPos4Color0Tex0
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