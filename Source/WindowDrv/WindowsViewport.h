#pragma once

#include <SDL.h>

class DLL_EXPORT UWindowsViewport : public UViewport
{
	DECLARE_CLASS(UWindowsViewport, UViewport, CLASS_Transient)
	DECLARE_WITHIN(UWindowsClient)

	UWindowsViewport();

	// From UObject
	virtual void Destroy() override;
	virtual void ShutdownAfterError() override;

	// UViewport interface.
	virtual UBOOL Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData = NULL, INT* HitSize = 0) override;
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual UBOOL ResizeViewport(DWORD BlitFlags, INT NewX = INDEX_NONE, INT NewY = INDEX_NONE, INT NewColorBytes = INDEX_NONE) override;
	virtual UBOOL IsFullscreen() override;
	virtual void Unlock( UBOOL Blit ) override;
	virtual void Repaint( UBOOL Blit ) override;
	virtual void SetModeCursor() override;
	virtual void UpdateWindowFrame() override;
	virtual void OpenWindow( DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY ) override;
	virtual void CloseWindow() override;
	virtual void UpdateInput( UBOOL Reset ) override;
	virtual void* GetWindow() override;
	virtual void SetMouseCapture( UBOOL Capture, UBOOL Clip, UBOOL FocusOnly ) override;
protected:
	void TryCreateRenderDevice(const TCHAR* ClassName, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen);
private:
	SDL_Window* m_SDLWindow = nullptr;
	HWND m_HWND = 0;
};

