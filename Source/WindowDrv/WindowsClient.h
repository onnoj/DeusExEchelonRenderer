#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

class DLL_EXPORT UWindowsClient : public UClient, public FNotifyHook
{
	DECLARE_CLASS(UWindowsClient, UClient, CLASS_Transient | CLASS_Config)

	UWindowsClient();
	void StaticConstructor();

	virtual void NotifyDestroy(void* Src) override;

	// From UObject
	virtual void Destroy() override;
	virtual void PostEditChange() override;
	virtual void ShutdownAfterError() override;

	// From UClient interface.
	virtual void Init(UEngine* InEngine) override;
	virtual void ShowViewportWindows( DWORD ShowFlags, INT DoShow ) override;
	virtual void EnableViewportWindows( DWORD ShowFlags, INT DoEnable ) override;
	virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar = *GLog) override;
	virtual void Tick() override;
	virtual void MakeCurrent( UViewport* InViewport ) override;
	virtual class UViewport* NewViewport(const FName Name)  override;

	bool GetStartupFullscreen() const { return m_StartupFullscreen;  }
	const SDL_SysWMinfo& GetSDLWMInfo() const { return m_SDLInfo; }
private:
	UBOOL m_StartupFullscreen = false;
	SDL_SysWMinfo m_SDLInfo{};
};


