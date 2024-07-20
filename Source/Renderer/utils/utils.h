#pragma once

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