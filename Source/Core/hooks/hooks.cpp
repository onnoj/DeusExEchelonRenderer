#include "DeusExEchelonCore_PCH.h"
#pragma hdrstop

#include "hooks.h"

namespace Hooks
{
  namespace UGameEngineCallbacks
  {
    NotifyLevelChangeMap OnNotifyLevelChange;
    TickMap OnTick;
  }
}
