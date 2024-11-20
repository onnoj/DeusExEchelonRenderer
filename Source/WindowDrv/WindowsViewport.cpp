#include "DeusExEchelonRendererWindowDrv_PCH.h"
#pragma hdrstop

#include <SDL.h>
#include <SDL_syswm.h>

#include "WindowsClient.h"
#include "WindowsViewport.h"


UWindowsViewport::UWindowsViewport() : UViewport()
{
  // This constructor originally set up the UWindowsViewport instance. It 
  // allocated memory for the window and initialized various member variables, 
  // such as Status and pointers for the keyboard and mouse. It also determined 
  // the color depth based on the screen's pixel format and set up standard cursors 
  // for different modes. It included error handling and ensured that any desktop 
  // resources were properly released after being used.
}

void  UWindowsViewport::Destroy()
{
  // This function was responsible for cleaning up resources when the viewport 
  // was destroyed. It called the base class's Destroy method, shut down 
  // DirectInput for the keyboard and mouse, freed any temporary screen memory, 
  // and safely deleted the window associated with the viewport. It ensured that 
  // all allocated resources were properly released to prevent memory leaks.
};

void  UWindowsViewport::ShutdownAfterError()
{
  // This function handled the cleanup process after an error occurred. It was 
  // designed to release any DirectDraw buffers and destroy the viewport window, 
  // ensuring that resources were not left dangling. It also called the base 
  // class method to handle any additional shutdown procedures that might be necessary.
};

//
// Lock the viewport window and set the appropriate Screen and RealScreen fields
// of Viewport. Returns 1 if locked successfully, 0 if failed. Note that a
// lock failing is not a critical error; it's a sign that a DirectDraw mode
// has ended or the user has closed a viewport window.
UBOOL UWindowsViewport::Lock(FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData /*= NULL*/, INT* HitSize /*= 0*/)
{
  /*
  The function was designed to lock the viewport window, ensuring that rendering could take place 
  safely. It returned a boolean indicating whether the lock was successful. A failure to lock was not considered 
  a critical error, but rather an indication that a DirectDraw mode had ended or that the user 
  had closed the viewport window.

  Initially, the function checked if the window was lockable by verifying the window handle, 
  the hold count, and the dimensions of the viewport. If any conditions failed, it returned 0.

  Next, it set the stride of the rendering buffer to the width of the viewport. If the viewport 
  was set to use DirectDraw, it attempted to lock the back buffer for rendering. 
  It first checked if the front buffer was lost and attempted to restore it if necessary. 
  Failure to restore led to logging an error and resizing the viewport to a DIB section.

  Upon successfully locking the back buffer, the function populated a surface description structure 
  with the dimensions of the buffer and retrieved the pointer to the surface. It adjusted the stride 
  based on the pitch provided in the surface description.

  If the viewport was set to use a DIB section instead of DirectDraw, it ensured that the screen pointer 
  was valid, indicating that it could safely write to the rendering buffer.

  Finally, after handling any potential locking issues, the function called the superclass's 
  lock function to finalize the locking process. It recorded the elapsed drawing time using the 
  `clock` and `unclock` macros to monitor rendering performance.

  This careful management of viewport locking ensured smooth rendering and responsiveness in the application.
  */
  return Super::Lock(FlashScale,FlashFog,ScreenClear,RenderLockFlags,HitData,HitSize);;
}

UBOOL UWindowsViewport::Exec(const TCHAR* Cmd, FOutputDevice& Ar) 
{
  // This method processed command-line inputs directed at the viewport. 
  // It handled various commands such as ending or toggling fullscreen mode, 
  // retrieving the current screen resolution and color depth, and adjusting 
  // display preferences. Each command was parsed, and corresponding actions 
  // were performed to reflect the user's requests within the viewport.

  if( UViewport::Exec( Cmd, Ar ) )
  {
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("EndFullscreen")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("ToggleFullscreen")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("GetCurrentRes")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("GetCurrentColorDepth")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("GetColorDepths")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("GetCurrentRenderDevice")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("GetRes")) )
  {
    //NOT IMPLEMENTED
    Ar.Log( TEXT("320x240 400x300 512x384 640x480 800x600") );
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("SetRes")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }
  else if( ParseCommand(&Cmd,TEXT("Preferences")) )
  {
    //NOT IMPLEMENTED
    return 1;
  }

  return 0;
}

UBOOL UWindowsViewport::ResizeViewport(DWORD BlitFlags, INT NewX /*= INDEX_NONE*/, INT NewY /*= INDEX_NONE*/, INT NewColorBytes /*= INDEX_NONE*/) {
  /*
  The function was designed to resize the viewport based on specified parameters such as new dimensions and color depth.
  It initially checked the blitting flags to determine the appropriate rendering method, handling cases for temporary viewports and 
  availability of DirectDraw. When switching from fullscreen to windowed mode, it ensured that any necessary render device changes were applied,
  and if the render device was exclusive to fullscreen, it switched to a software renderer.

  The function then stored the current viewport, accepting default parameters for the new dimensions and color bytes if they were unspecified.
  It handled shutdown procedures for existing rendering sessions and ensured proper resource cleanup, including the release of back and front buffers
  if DirectDraw was being used.

  Next, it calculated the new window rectangle and aligned the new dimensions to ensure they met the necessary requirements.
  If the viewport was currently in fullscreen mode, it executed procedures to exit fullscreen, restoring window parameters and releasing hotkeys that inhibited standard window operations.

  If transitioning into fullscreen, it saved necessary window parameters and modified the window style to eliminate borders and menus for a more immersive experience.
  The function also handled the display method by attempting to match the closest available DirectDraw mode, or creating a DIB section if that method was specified.

  Throughout the process, the function ensured that it maintained the correct rendering device settings and updated audio viewport information as needed. 
  It also managed mouse capture and drag settings based on the current state of the viewport (fullscreen or windowed).

  Finally, it saved the new configuration details, updating either fullscreen or windowed settings based on the transition, and ensured that any changes were persisted in the client configuration.
  The function concluded by returning success status indicating whether the resize operation was executed successfully.
  */

  // Set new info.
  DWORD OldBlitFlags = BlitFlags;
  BlitFlags          = BlitFlags & ~BLIT_ParameterFlags;
  SizeX              = NewX;
  SizeY              = NewY;
  ColorBytes         = NewColorBytes ? NewColorBytes : ColorBytes;

  return TRUE;
}

UBOOL UWindowsViewport::IsFullscreen() 
{
  // This method provided information on whether the viewport was currently 
  // operating in fullscreen mode. It checked the appropriate flags and returned 
  // a boolean value indicating the viewport's state, facilitating user interface 
  // adjustments and behavior based on the viewport's configuration.
  return FALSE;
}

void  UWindowsViewport::Unlock(UBOOL Blit)
{
  // This function was responsible for unlocking the viewport and managing the rendering process.
  // Initially, it retrieved the associated UWindowsClient object, ensuring that the hold count was zero.
  // It reset the drawing cycles to zero and recorded the current time for draw cycle performance measurement.
  Super::Unlock(Blit);
}

void  UWindowsViewport::Repaint(UBOOL Blit)
{
  // This function was responsible for refreshing the viewport's display. It 
  // called the rendering engine to draw the current scene onto the viewport, 
  // ensuring that any updates or changes were reflected visually for the user.
}

void  UWindowsViewport::SetModeCursor()
{
  // This function originally handled the setting of the mouse cursor based on 
  // the current mode of the viewport or editor. It ensured that the cursor 
  // reflected the active operation or task, changing to a wait cursor when 
  // necessary to indicate ongoing processes, thereby improving user experience.
}

void  UWindowsViewport::UpdateWindowFrame()
{
  /*
  The function was responsible for updating the user interface of the viewport window. 
  It first checked if the function could proceed by confirming that the viewport was indeed a window 
  and was not in fullscreen or temporary mode. If any of these conditions were met, the function exited early.

  Next, the function set the viewport window's title to reflect the current resolution or view type. 
  If the application was not in editor mode or if the player controls were visible, 
  it displayed the product name. Otherwise, it determined the appropriate title based on the rendering map 
  (renamed as `RendMap`), setting titles for various views such as perspective, orthographic views, 
  or defaulting to a generic view title.

  The function also managed the window menu. If the menu should be shown and the viewport was not in fullscreen, 
  it loaded a localized menu if it hadn't already been loaded, and updated it with the current rendering options.
  When the menu was set, it checked the rendering mode options and updated the menu items to reflect 
  the current rendering settings by marking the appropriate items as checked or unchecked.

  It proceeded to check and update various attributes that could be shown in the viewport, 
  such as brushes, backdrops, coordinates, moving brushes, and paths, by checking the corresponding flags 
  in the actor's show flags.

  Afterward, the function evaluated the actor display options, determining how to represent actor icons 
  and radii in the viewport based on the current show flags. It set the appropriate menu items 
  to checked or unchecked according to the state of these filters.

  Finally, the function checked the color depth settings and updated the menu accordingly, marking 
  the appropriate items for 16-bit or 32-bit color depths. It concluded by sending a character message 
  to the parent window to ensure it was refreshed, thereby maintaining synchronization between the viewport 
  and its parent interface.
  */
}

//
// Try switching to a new rendering device.
//
void UWindowsViewport::TryCreateRenderDevice( const TCHAR* ClassName, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen )
{
  // Shut down current rendering device.
  if( this->RenDev )
  {
    this->RenDev->Exit();
    delete this->RenDev;
    this->RenDev = nullptr;
  }

  UClass* renderClass = UObject::StaticLoadClass( URenderDevice::StaticClass(), NULL, ClassName, NULL, 0, NULL );
  if( renderClass )
  {
    this->RenDev = ConstructObject<URenderDevice>( renderClass, this );
    if( this->RenDev->Init( this, NewX, NewY, NewColorBytes, Fullscreen ) )
    {
      if( GIsRunning )
        Actor->GetLevel()->DetailChange( this->RenDev->HighDetailActors );
    }
    else
    {
      debugf( NAME_Log, LocalizeError("Failed3D") );
      delete this->RenDev;
      this->RenDev = nullptr;
    }
  }
  GRenderDevice = this->RenDev;
}

void  UWindowsViewport::OpenWindow(DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY)
{
  // This function originally managed the creation and opening of the viewport's 
  // window. It set up the necessary parameters and styles, allocated any required 
  // memory for a temporary viewport if needed, and established the rendering 
  // device. It also ensured that the window was displayed correctly according to 
  // the specified attributes, updating the window's position and size as necessary.

  UWindowsClient* C = GetOuterUWindowsClient();
  const bool isFullscreen = C->GetStartupFullscreen();
  INT NewColorBytes = isFullscreen ? C->FullscreenColorBits/8 : ColorBytes;
  if( NewX==INDEX_NONE )
    NewX = isFullscreen ? C->FullscreenViewportX : C->WindowedViewportX;
  if( NewY==INDEX_NONE )
    NewY = isFullscreen ? C->FullscreenViewportY : C->WindowedViewportY;

  NewX = max(NewX, 1);
  NewY = max(NewY, 1);

  m_SDLWindow = SDL_CreateWindow("Deus Ex", OpenX, OpenY, NewX, NewY, SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  
  SDL_SysWMinfo sdlWMInfo = GetOuterUWindowsClient()->GetSDLWMInfo();
  SDL_GetWindowWMInfo(m_SDLWindow, &sdlWMInfo);
  m_HWND = sdlWMInfo.info.win.window;

  TryCreateRenderDevice( TEXT("ini:Engine.Engine.GameRenderDevice"), NewX, NewY, NewColorBytes, isFullscreen );

  check(RenDev != nullptr); //RenDev needs to be set at the end of this.
}

void  UWindowsViewport::CloseWindow()
{
  // This function was responsible for closing the viewport's window. It checked 
  // the current status of the viewport and, if it was in a normal state, initiated 
  // the destruction of the associated window. This process was crucial for ensuring 
  // that resources were freed correctly and that the viewport was closed cleanly.
  SDL_DestroyWindow(m_SDLWindow);
  m_SDLWindow = nullptr;
}

void  UWindowsViewport::UpdateInput(UBOOL Reset)
{
  /*
  The function was responsible for updating input events for the viewport, processing 
  mouse, joystick, and keyboard inputs. It started by initializing a processed array to track 
  which input events had already been handled.

  For joystick input, the function checked if the joystick had buttons available. 
  If so, it retrieved the joystick state using the `joyGetPosEx` function. 
  After successfully fetching the joystick data, it processed button presses and releases, 
  mapping joystick button states to internal input keys. 

  The function also handled joystick axes movements, passing the positional data for the joystick 
  axes to the application through `JoystickInputEvent`. It accounted for various capabilities of the joystick, 
  including X, Y, Z, and rotation axes, along with the POV (point of view) hat switches, 
  updating the respective internal input states.

  Next, the function dealt with mouse input through DirectInput. It checked if the mouse was acquired 
  and retrieved its state. If the input was lost, it reacquired the mouse device. 
  After acquiring the state, it computed the difference in mouse movement from the last recorded position 
  and generated corresponding input events for mouse movements and wheel scrolling.

  For buffered mouse clicks, the function read input data from the mouse buffer, checking the state of 
  the left, right, and middle mouse buttons. It generated press and release events based on the button states.

  Finally, the function processed keyboard input. It reset the keyboard state if the viewport had focus. 
  For each possible key, it checked if the key was not already processed. 
  If the key was not pressed and it was in reset mode, it checked the state of the key and generated 
  a press event if it was down. Conversely, if the key was pressed but not detected as down, 
  it generated a release event.

  This comprehensive handling ensured that all user inputs were accurately reflected in the viewport's state.
  */

  SDL_Event event;
  while (SDL_PollEvent(&event) != 0)
  {
    switch (event.type)
    {
      case SDL_QUIT:
      {
        //window closed...
        return;
      } break;
      case SDL_RENDER_DEVICE_RESET:
      {
        int x = 1;
      } break;
    }
  }

  auto causeInputEvent = [&](INT iKey, EInputAction Action, FLOAT Delta=0.0f) -> UBOOL
    {
      // Route to engine if a valid key; some keyboards produce key
      // codes that go beyond IK_MAX.
      if (iKey >= 0 && iKey < IK_MAX)
        return GetOuterUWindowsClient()->Engine->InputEvent(this, (EInputKey)iKey, Action, Delta);
      else
        return 0;
    };

  int keyStateArrayLength = 0;
  SDL_PumpEvents();
  const uint8_t* keyStates = SDL_GetKeyboardState(&keyStateArrayLength);
  for (int i = 0; i < keyStateArrayLength; i++)
  {
    const auto pressed = (keyStates[i] != 0);
    SDL_Scancode scanCode = SDL_Scancode(i);
    UINT virtualKeyCode = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
    causeInputEvent(virtualKeyCode, pressed ? IST_Press : IST_Release);
  }


  //mouse:
  static int lastMouseX = 0, lastMouseY = 0;
  int mouseX, mouseY;
  auto buttonMask = SDL_GetMouseState(&mouseX, &mouseY);

  int mouseDiffX = mouseX - lastMouseX;
  if (mouseDiffX)
  {
    causeInputEvent(IK_MouseX, IST_Axis, +mouseDiffX);
  }
  lastMouseX = mouseX;

  int mouseDiffY = mouseY - lastMouseY;
  if (mouseDiffY)
  {
    causeInputEvent(IK_MouseY, IST_Axis, -mouseDiffY);
  }
  lastMouseY = mouseY;
  GetOuterUWindowsClient()->Engine->MouseDelta( this, 0, mouseDiffX, mouseDiffY );
  causeInputEvent(IK_LeftMouse, (buttonMask&SDL_BUTTON_LMASK) ? IST_Press : IST_Release );
  causeInputEvent(IK_RightMouse, (buttonMask&SDL_BUTTON_RMASK) ? IST_Press : IST_Release );
}

void* UWindowsViewport::GetWindow() {
  return m_HWND;
}

void  UWindowsViewport::SetMouseCapture(UBOOL Capture, UBOOL Clip, UBOOL FocusOnly)
{
  // This function managed the capturing of the mouse cursor within the 
  // viewport. It handled whether the cursor should be clipped or captured, 
  // brought the window to the foreground when necessary, and ensured that 
  // the cursor's state was correctly reflected according to the current 
  // viewport conditions and user actions.
}

