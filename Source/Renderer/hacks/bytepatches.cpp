#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hacks.h"

namespace
{
  void* galaxyMalloc(size_t Size);
  decltype(&galaxyMalloc) originalGalaxyMalloc;
  void* galaxyMalloc(size_t Size)
  {
    return originalGalaxyMalloc(Size * 2);
  }
}

namespace
{
  struct Patch
  {
    uint32_t offset;
    std::vector<BYTE> newBytes;
  };

  template <int TSz>
  void PatchCode(Patch (&pPatchData)[TSz], const char* pModuleName)
  {
    uint32_t moduleBase = reinterpret_cast<uint32_t>(GetModuleHandleA(pModuleName));
    DWORD oldProtect = 0;
    for (auto [offset, patch] : pPatchData)
    {
      const auto length = patch.size();
      if (VirtualProtect(LPVOID(moduleBase + offset), length * 2, PAGE_READWRITE, &oldProtect))
      {
        for (int i = 0; i < length; i++)
        {
          *reinterpret_cast<uint8_t*>(moduleBase + offset + i) = patch[i];
        }
        VirtualProtect(LPVOID(moduleBase + offset), length * 2, oldProtect, &oldProtect);
      }
    }
  }
}

void InstallBytePatches()
{
  static bool installed = false;
  if (!installed)
  {
    installed = true;

    // =========================================================================
    // render.dll — Backface Culling Disable + FTransform Outcode Zeroing
    // render.dll imagebase: 0x10B00000
    // =========================================================================
    //
    // SECTION A — Backface culling (6 patches, all JNZ/JZ → 6×NOP)
    //
    //   After each mesh triangle's winding is computed, the rasterizer tests
    //   the cross-product sign and the FPU status word to skip back-facing and
    //   mirror-reflected triangles respectively.  Both checks compile to a
    //   6-byte conditional jump (JNZ or JZ with a 4-byte rel32 displacement).
    //   Each is replaced with 6 NOPs to make every triangle unconditionally
    //   visible regardless of facing direction.
    //
    //   Without these patches mesh actors (characters, weapons, LAW missiles)
    //   lose roughly half their triangles.  The LAW missile exposes the crash
    //   case: all its triangles are culled before submission, producing an
    //   empty draw list that RTX Remix cannot handle.
    //
    // SECTION B — FTransform outcode zeroing (6 patches)
    //
    //   ComputeOutcode builds per-vertex frustum-rejection flags (FVF outcodes)
    //   stored at FTransform+0x0C.  The pattern at every site is:
    //
    //     mov  al, [frustumOutcodeTable + esi]   ; per-axis visibility bits
    //     or   al, bl                             ; OR with accumulated flags  ← patch target
    //     mov  [ecx+0Ch], al                      ; store to FTransform.Flags
    //
    //   Patch bytes C6 41 0C 00 90 overwrite the last two instructions:
    //
    //     mov  byte ptr [ecx+0Ch], 0    ; force outcode to 0 (fully inside frustum)
    //     nop                           ; pad for the displaced byte
    //
    //   Forcing the outcode to 0 prevents the subsequent "all vertices outside
    //   same half-space" rejection from discarding world-space geometry that
    //   the software frustum would otherwise cull.
    // =========================================================================
    if (true)
    {
      Patch patches[] = {
#if 1
        // -----------------------------------------------------------------------
        // A1. URender::DrawLodMesh — LOD path (UnMeshRnLOD.h:454)
        //     Cross-product sign test + FPU mirror test for LOD-selected meshes.
        //
        //   10B0FF93  test bl, al
        //   10B0FF95  jnz  loc_10B1010E   ← 6-byte JNZ  PATCHED → 6×NOP
        //
        //   10B0FFC4  fnstsw ax
        //   10B0FFC6  test ah, 1
        //   10B0FFC9  jz   loc_10B1010E   ← 6-byte JZ   PATCHED → 6×NOP
        // -----------------------------------------------------------------------
        {0xFF95, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // backface JNZ  → NOP×6
        {0xFFC9, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // mirror   JZ   → NOP×6

        // -----------------------------------------------------------------------
        // A2. URender::DrawLodMesh — no-LOD path (UnMeshRnLOD.h:517)
        //     Same pair of checks in the variant that skips LOD selection.
        //
        //   10B10314  test [esi+0Ch], al
        //   10B10317  jnz  loc_10B10496   ← PATCHED → 6×NOP
        //
        //   10B10348  test ah, 1
        //   10B1034B  jz   loc_10B10496   ← PATCHED → 6×NOP
        // -----------------------------------------------------------------------
        {0x10317, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // backface JNZ → NOP×6
        {0x1034B, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // mirror   JZ  → NOP×6

        // -----------------------------------------------------------------------
        // A3. URender::DrawMesh (UnMeshRn.cpp:499)
        //     Same pair of checks for the non-LOD mesh path used by most actors.
        //
        //   10B0DC7E  fstp dword ptr [ecx+8]
        //   10B0DC81  jnz  loc_10B0DD8E   ← PATCHED → 6×NOP
        //
        //   10B0DCB8  test ah, 1
        //   10B0DCBB  jz   loc_10B0DD8E   ← PATCHED → 6×NOP
        // -----------------------------------------------------------------------
        {0xDC81, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // backface JNZ → NOP×6
        {0xDCBB, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }},  // mirror   JZ  → NOP×6
#endif

#if 0
        // -----------------------------------------------------------------------
        // B0. URender::DrawLodMesh — WeaponOutcode path (UnMeshRnLOD.h:170)
        //     Outcode zeroing for a separate coordinate path, possibly weapon
        //     attachments.  Disabled: coordinate handling here differs from the
        //     main mesh path and zeroing it caused rendering artefacts.
        //
        //   10B0F071  mov [ecx+0Ch], al   ← WOULD BE: mov [ecx+0Ch], 0 + NOP
        // -----------------------------------------------------------------------
        {0x0F071, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},
#endif

        // -----------------------------------------------------------------------
        // B1. URender::DrawLodMesh — MeshOutcode (UnMeshRnLOD.h:176)
        //
        //   10B0F18E  or   al, bl
        //   10B0F190  mov  [ecx+0Ch], al  ← PATCHED → mov [ecx+0Ch], 0 + NOP
        // -----------------------------------------------------------------------
        {0x0F190, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B2. URender::DrawMesh — ComputeOutcode (UnMeshRn.cpp:354)
        //
        //   10B0D45E  or   al, bl
        //   10B0D460  mov  [ecx+0Ch], al  ← PATCHED
        // -----------------------------------------------------------------------
        {0x0D460, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B3. URender::RenderSubsurface — per-vertex outcode (UnMeshRn.cpp:162)
        //
        //   10B0C425  or   al, bl
        //   10B0C427  mov  [ecx+0Ch], al  ← PATCHED
        // -----------------------------------------------------------------------
        {0x0C427, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B4. URender::DrawMesh — second outcode site
        //
        //   10B0E241  or   al, bl
        //   10B0E243  mov  [ecx+0Ch], al  ← PATCHED
        // -----------------------------------------------------------------------
        {0x0E243, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B5. URender::DrawLodMesh — second outcode site
        //     (Redundant with B1 above — both target RVA 0x0F190.  Harmless
        //     to apply twice.)
        // -----------------------------------------------------------------------
        {0x0F190, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B6. URender::DrawLodMesh — third outcode site
        //
        //   10B109E1  or   al, bl
        //   10B109E3  mov  [ecx+0Ch], al  ← PATCHED
        // -----------------------------------------------------------------------
        {0x109E3, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

        // -----------------------------------------------------------------------
        // B7. URender::ClipDecal — outcode site
        //
        //   10B1C6FF  or   al, bl
        //   10B1C701  mov  [ecx+0Ch], al  ← PATCHED
        // -----------------------------------------------------------------------
        {0x1C701, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},

#if 0
        // -----------------------------------------------------------------------
        // B8. URender::ClipBspSurf — outcode site (disabled: causes flickering)
        //     Zeroing the outcode here causes BSP surfaces to flicker, likely
        //     because ClipBspSurf feeds the span-buffer rasterizer which has a
        //     hard polygon-count limit.  With no frustum rejection the limit is
        //     exceeded and polygons are dropped inconsistently each frame.
        // -----------------------------------------------------------------------
        {0x14EE6, { 0xC6, 0x41, 0x0C, 0x00, 0x90 }},
#endif
      };

      PatchCode(patches, "render.dll");
    }

    // =========================================================================
    // core.dll — Disable asserts in UObject::UnhashObject
    // core.dll imagebase: 0x10100000
    // =========================================================================
    //
    // UnhashObject walks GObjHash to remove an object and asserts two
    // postconditions after the walk:
    //
    //   "Removed!=0" — object must have been found in the table
    //   "Removed==1" — object must appear exactly once
    //
    // The renderer zeroes Actor->Location and Frame->Coords in DrawMesh to
    // force world-space geometry submission.  Zeroing these fields corrupts the
    // hash bucket key, so UnhashObject can no longer locate the actor — firing
    // the first assert — or finds stale duplicate entries, firing the second.
    //
    // Patch: both conditional jumps are changed to unconditional JMP (0xEB),
    // making each assert block unreachable regardless of the removed count.
    //
    //   101577B5  test esi, esi
    //   101577B7  jnz  short loc_101577D0   ← 75 17  PATCHED → EB 17
    //   101577B9  ; ... push args ...
    //   101577C8  call appFailAssert          ; "Removed!=0"
    //
    //   101577D0  cmp  esi, 1
    //   101577D3  jz   short loc_101577EC   ← 74 17  PATCHED → EB 17
    //   101577D5  ; ... push args ...
    //   101577E4  call appFailAssert          ; "Removed==1"
    // =========================================================================
    if (true)
    {
      uint32_t coreModule = reinterpret_cast<uint32_t>(GetModuleHandleA("core.dll"));
      uint32_t offsets[] = { 0x577b7, 0x577d3 };
      DWORD oldProtect = 0;
      for (auto offset : offsets)
      {
        if (VirtualProtect(LPVOID(coreModule + offset), 6 * 2, PAGE_READWRITE, &oldProtect))
        {
          *reinterpret_cast<uint8_t*>(uint32_t(coreModule) + offset + 0) = 0xeb;
          VirtualProtect(LPVOID(coreModule + offset), 6 * 2, oldProtect, &oldProtect);
        }
      }
    }

    // =========================================================================
    // render.dll — RenderSubsurface Clipping Disable + DrawLodMesh Outcode Gate
    // render.dll imagebase: 0x10B00000
    // =========================================================================
    //
    // RenderSubsurface is the per-triangle software rasterization entry point
    // for BSP surfaces.  It applies four successive culling/clipping passes
    // after per-vertex outcodes are computed.  All four are disabled so that
    // world-space BSP geometry reaches the D3D9 submission path unclipped.
    //
    // PASS 1 — "All outside same plane" outcode rejection (0xC52A–0xC52B)
    //
    //   If the combined outcode of all three vertices is nonzero (every vertex
    //   is outside the same frustum half-space), the triangle is trivially
    //   invisible and skipped.  Two NOPs replace the 2-byte conditional jump.
    //
    //   10B0C528  test dl, al            ; dl = combined outcode of all 3 verts
    //   10B0C52A  jnz  short loc_...     ← 75 xx  PATCHED → NOP NOP
    //
    // PASS 2 — Backface rejection (0xC571)
    //
    //   FPU status word test to skip back-facing BSP triangles.  Patching the
    //   JZ to an unconditional JMP always falls through to the front-face path.
    //
    //   10B0C56E  test ah, 41h
    //   10B0C571  jz   short loc_...     ← 74 xx  PATCHED → EB xx (JMP)
    //
    // PASS 3 — Per-vertex clip-loop gate (0xC592–0xC597)
    //
    //   A JZ branches past the per-vertex clip loop when the combined outcode
    //   is zero (all vertices inside, no clipping needed).  Making it an
    //   unconditional JMP always skips the clip loop, preventing any vertex
    //   from being modified or rejected by the clipping code.
    //
    //   Original: 0F 84 78 05 00 00  (JZ rel32, 6 bytes, target = 10B0CB10)
    //   After:    E9 79 05 00 00 90  (JMP rel32, 5 bytes, same target)
    //
    //   The displacement is incremented by 1 (0x578 → 0x579) to compensate
    //   for the 1-byte difference in instruction length (JZ=6, JMP=5).
    //   Byte 0xC596 (value 0x00) is not written; 0xC597 is overwritten with NOP.
    //
    //   10B0C58F  mov  [ebp+SubCount+3], al  ; combined outcode of all 3 verts
    //   10B0C592  jz   loc_10B0CB10          ← PATCHED → JMP loc_10B0CB10
    //
    // PASS 4a — Near-plane clipping (0xCB32)
    //
    //   RenderSubsurface computes a dot product to detect vertices behind the
    //   near plane.  One factor is NearClip.Z, loaded via fmul [esi+2Ch].
    //
    //   IMPORTANT: this patch targets byte 1 (the ModRM byte) of a 3-byte
    //   x87 memory-reference instruction, not an opcode boundary:
    //
    //     10B0CB31  D8 4E 2C   fmul dword ptr [esi+2Ch]
    //                   ↑ RVA 0xCB32 — PATCHED: 0x4E → 0xEB
    //
    //   After the patch the CPU decodes: D8 EB = fsubr st(3)  (2-byte FPU
    //   register instruction).  The following 0x2C byte becomes a misaligned
    //   orphan decoded as part of the next instruction.  The net effect is
    //   that the NearClip.Z dot-product component is replaced with a
    //   register subtraction, corrupting the accumulator so the "behind near
    //   plane" threshold is never reached and near-plane clipping never fires.
    //
    // PASS 4b — Rasterization clipping (0xCB9A)
    //
    //   After the near-plane pass, an edge-clipping loop is gated by a JGE
    //   that skips to the post-clip submission path.  Patching to JMP makes
    //   the skip unconditional, bypassing the rasterizer's viewport clipping.
    //
    //   10B0CB98  cmp  edi, ecx
    //   10B0CB9A  jge  short loc_...     ← 7D xx  PATCHED → EB xx (JMP)
    //
    // DrawLodMesh outcode gate (0xFC54–0xFC59)
    //
    //   At the per-triangle entry point of DrawLodMesh, the accumulated
    //   outcode for the triangle's three vertices is compared and the triangle
    //   skipped if nonzero.  Six NOPs replace the 6-byte conditional jump.
    //
    //   10B0FC44  mov  ecx, [ebp+MeshOutcode]
    //   10B0FC48  cmp  ecx, eax
    //   10B0FC54  jnz  loc_10B104A2         ← 0F 85 xx xx xx xx  PATCHED → 6×NOP
    // =========================================================================
    if (true)
    {
      uint32_t renderModule = reinterpret_cast<uint32_t>(GetModuleHandleA("render.dll"));
      constexpr std::pair<uint32_t, uint32_t> patches[] = {
        // Pass 1: outcode same-plane rejection (JNZ → NOP NOP)
        {0x0000C52A, 0x90},
        {0x0000C52B, 0x90},

        // Pass 2: backface rejection (JZ → JMP)
        {0x0000C571, 0xEB},

        // Pass 3: clip-loop gate (JZ rel32 → JMP rel32, displacement +1, trailing NOP)
        {0x0000C592, 0xE9},  // JMP rel32 opcode
        {0x0000C593, 0x79},  // displacement byte 0 (was 0x78, +1 for length difference)
        {0x0000C594, 0x05},  // displacement byte 1
        {0x0000C595, 0x00},  // displacement byte 2
        // 0xC596 not written (original 0x00 retained as byte 3 of displacement)
        {0x0000C597, 0x90},  // NOP over trailing byte of original 6-byte JZ

        // Pass 4a: near-plane clipping — patches ModRM byte of fmul [esi+2Ch]
        // D8 4E 2C → D8 EB (fsubr st(3)), corrupting the dot-product accumulator
        {0x0000CB32, 0xEB},

        // Pass 4b: rasterization clipping (JGE → JMP)
        {0x0000CB9A, 0xEB},

        // DrawLodMesh per-triangle outcode gate (JNZ → 6×NOP)
        {0x0000FC54, 0x90},
        {0x0000FC55, 0x90},
        {0x0000FC56, 0x90},
        {0x0000FC57, 0x90},
        {0x0000FC58, 0x90},
        {0x0000FC59, 0x90},
      };
      DWORD oldProtect = 0;
      for (auto patch : patches)
      {
        if (VirtualProtect(LPVOID(renderModule + patch.first), 4 * 2, PAGE_READWRITE, &oldProtect))
        {
          *reinterpret_cast<uint8_t*>(uint32_t(renderModule) + patch.first) = patch.second;
          VirtualProtect(LPVOID(renderModule + patch.first), 4 * 2, oldProtect, &oldProtect);
        }
      }
    }

    // =========================================================================
    // render.dll — Expand PointCache size and update MAX_POINTS bounds checks
    // render.dll imagebase: 0x10B00000
    // =========================================================================
    //
    // URender::PointCache is a flat array of FStampedPoint (8 bytes each) pre-
    // allocated at startup with MAX_POINTS entries.  With backface culling and
    // frustum rejection disabled, every mesh vertex in the scene — including
    // all back-facing triangles and off-screen actors — passes through the
    // transform pipeline and consumes a cache slot.  The original size is
    // insufficient.
    //
    // Fix part 1 — C++ re-allocation:
    //   The original PointCache and DynamicsCache allocations are deleted and
    //   replaced with double-sized ones.  The frame memory pools (GDynMem,
    //   GSceneMem, VectorMem) are also doubled via their Init() calls.
    //
    // Fix part 2 — Byte patches to keep in-binary bounds checks consistent:
    //
    //   RVA 0x1354D+1 (URender::Init) — PointCache stamp loop upper bound
    //     Init stamps every PointCache slot with the current Stamp value.  The
    //     loop bound is a 4-byte immediate in a CMP instruction:
    //
    //       10B1354B  xor  eax, eax
    //       10B1354D  cmp  eax, 1F400h     ← immediate at RVA 0x1354E  PATCHED
    //       10B13552  jge  short done
    //       10B13554  mov  [ecx+eax*8+4], edx
    //       10B13558  inc  eax
    //       10B13559  jmp  short 10B1354D
    //
    //     The 4-byte immediate is overwritten with newMaxPoints so the stamp
    //     loop initialises every slot in the enlarged allocation.
    //
    //   RVA 0x1742E+6 (URender::OccludeBsp) — world point count sanity assert
    //     OccludeBsp asserts that the level geometry does not exceed MAX_POINTS:
    //
    //       10B1742E  cmp  dword ptr [esi+8Ch], 1F400h  ← immediate at RVA 0x17434  PATCHED
    //       10B17438  jle  short ok
    //       10B1743A  push 889h
    //       10B17444  push "Frame->Level->Model->Points.Num()<=MAX_POINTS"
    //       10B17449  call appFailAssert
    //
    //     The threshold is raised to newMaxPoints so large worlds don't trigger
    //     a false assert failure with the expanded cache.
    // =========================================================================
    if (true)
    {
      constexpr uint32_t newMaxPoints = MAX_POINTS * 2;
      constexpr uint32_t newMaxNodes = MAX_NODES * 2;
      delete(URender::PointCache);
      URender::PointCache = new(URender::FStampedPoint[newMaxPoints]);
      URender::DynamicsCache = new(TEXT("FDynamicsCache"))URender::FDynamicsCache[newMaxNodes];
      appMemzero(URender::DynamicsCache, newMaxNodes * sizeof(URender::FDynamicsCache));
      GDynMem.Init(655360 * 2);
      GSceneMem.Init(327680 * 2);
      URender::VectorMem.Init(163840 * 2);

      uint32_t module = reinterpret_cast<uint32_t>(GetModuleHandleA("render.dll"));
      const uint32_t offsets[] = { 0x1354d + 1, 0x1742e + 6 };  // immediate operand addresses
      DWORD oldProtect = 0;
      for (auto offset : offsets)
      {
        if (VirtualProtect(LPVOID(module + offset), 4, PAGE_READWRITE, &oldProtect))
        {
          auto& val = *reinterpret_cast<uint32_t*>(uint32_t(module) + offset + 0);
          val = newMaxPoints;
          VirtualProtect(LPVOID(module + offset), 4, oldProtect, &oldProtect);
        }
      }
    }

    // =========================================================================
    // galaxy.dll — Hijack malloc to double all allocation sizes
    // =========================================================================
    //
    // Galaxy (the Deus Ex audio/mixer library) routes internal allocations
    // through a non-virtual malloc wrapper.  The function pointer for that
    // malloc is stored in a data slot inside galaxy.dll; the call site at
    // RVA 0x20D81 is an indirect call whose 4-byte pointer operand (at
    // RVA 0x20D83 = 0x20D81+2) references the slot.
    //
    // The hijack reads the current pointer from that slot, saves it as
    // originalGalaxyMalloc, then replaces it with &galaxyMalloc — a wrapper
    // that doubles the requested size before forwarding to the original.
    // Doubling prevents sound buffers from overflowing when RTX Remix's
    // audio processing requests more space than the original allocation assumed.
    //
    // galaxy.dll may not be loaded at the time InstallBytePatches() runs, so
    // the swap is attempted on a background thread that polls until the module
    // appears.
    // =========================================================================
    if (g_options.galaxyMallocFix)
    {
      std::thread([]() {
        do
        {
          uint32_t module = reinterpret_cast<uint32_t>(GetModuleHandleA("galaxy.dll"));
          if (module != 0)
          {
            uint32_t** galaxyMallocPtr = reinterpret_cast<uint32_t**>(module + 0x20D81 + 2);
            uint32_t overridePtr = reinterpret_cast<uint32_t>(&galaxyMalloc);
            originalGalaxyMalloc = reinterpret_cast<decltype(originalGalaxyMalloc)>(**galaxyMallocPtr);
            **galaxyMallocPtr = overridePtr;
            return;
          }
          ::Sleep(1000);
        } while (true);
      }).detach();
    }
  }
}

void UninstallBytePatches()
{
  //nothing to do
}
