
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtensionCore.h
//  Programmer  :  Scott Martin
//  Description :  Core header for the Extension package and DLL
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#include "Engine.h"


// ----------------------------------------------------------------------
// Needed for all routines and classes visible outside the DLL

#ifndef EXTENSION_API
#define EXTENSION_API DLL_IMPORT
#endif

#include "ExtObject.h"

// ----------------------------------------------------------------------
// Extension methods in UnrealScript that can be called from C++

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern EXTENSION_API FName EXTENSION_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif

AUTOGENERATE_NAME(InitWindow)
AUTOGENERATE_NAME(DestroyWindow)
AUTOGENERATE_NAME(WindowReady)
AUTOGENERATE_NAME(DrawWindow)
AUTOGENERATE_NAME(PostDrawWindow)
AUTOGENERATE_NAME(ParentRequestedPreferredSize)
AUTOGENERATE_NAME(ParentRequestedGranularity)
AUTOGENERATE_NAME(ChildRequestedVisibilityChange)
AUTOGENERATE_NAME(ChildRequestedReconfiguration)
AUTOGENERATE_NAME(ChildRequestedShowArea)
AUTOGENERATE_NAME(ConfigurationChanged)
AUTOGENERATE_NAME(VisibilityChanged)
AUTOGENERATE_NAME(SensitivityChanged)
AUTOGENERATE_NAME(ChildAdded)
AUTOGENERATE_NAME(ChildRemoved)
AUTOGENERATE_NAME(DescendantAdded)
AUTOGENERATE_NAME(DescendantRemoved)
AUTOGENERATE_NAME(CursorRequested)
AUTOGENERATE_NAME(MouseMoved)
AUTOGENERATE_NAME(MouseEnteredWindow)
AUTOGENERATE_NAME(MouseLeftWindow)
AUTOGENERATE_NAME(FocusEnteredWindow)
AUTOGENERATE_NAME(FocusLeftWindow)
AUTOGENERATE_NAME(FocusEnteredDescendant)
AUTOGENERATE_NAME(FocusLeftDescendant)
AUTOGENERATE_NAME(Tick)
AUTOGENERATE_NAME(ButtonActivated)
AUTOGENERATE_NAME(ButtonActivatedRight)
AUTOGENERATE_NAME(ToggleChanged)
AUTOGENERATE_NAME(BoxOptionSelected)
AUTOGENERATE_NAME(ScalePositionChanged)
AUTOGENERATE_NAME(ScaleRangeChanged)
AUTOGENERATE_NAME(ScaleAttributesChanged)
AUTOGENERATE_NAME(ClipAttributesChanged)
AUTOGENERATE_NAME(ListRowActivated)
AUTOGENERATE_NAME(ListSelectionChanged)
AUTOGENERATE_NAME(ClipPositionChanged)
AUTOGENERATE_NAME(FilterChar)
AUTOGENERATE_NAME(TextChanged)
AUTOGENERATE_NAME(EditActivated)
AUTOGENERATE_NAME(CalcView)
AUTOGENERATE_NAME(RawMouseButtonPressed)
AUTOGENERATE_NAME(MouseButtonPressed)
AUTOGENERATE_NAME(MouseButtonReleased)
AUTOGENERATE_NAME(RawKeyPressed)
AUTOGENERATE_NAME(VirtualKeyPressed)
AUTOGENERATE_NAME(KeyPressed)
AUTOGENERATE_NAME(AcceleratorKeyPressed)
AUTOGENERATE_NAME(StyleChanged)
AUTOGENERATE_NAME(ComputerStart)
AUTOGENERATE_NAME(ComputerInputFinished)
AUTOGENERATE_NAME(ComputerFadeOutCompleted)

#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif


// ----------------------------------------------------------------------
// Yeah, I know this is weird, but Unreal does it this way...

#ifndef _EXTENSION_CORE_H_
#define _EXTENSION_CORE_H_


// ----------------------------------------------------------------------
// Enumerations used for intrinsic routines in the Extension DLL

enum EExtensionIntrinsics
{
	// XExtensionObject
	EXTENSION_ObjStringToName              = 1024,

	// APlayerPawnExt
	EXTENSION_PreRenderWindows             = 1050,
	EXTENSION_PostRenderWindows            = 1051,
	EXTENSION_InitRootWindow               = 1052,
	EXTENSION_InitLevelActor               = 1053,

	// XFlagBase
	EXTENSION_FlagBaseSetBool              = 1100,
	EXTENSION_FlagBaseSetByte              = 1101,
	EXTENSION_FlagBaseSetInt               = 1102,
	EXTENSION_FlagBaseSetFloat             = 1103,
	EXTENSION_FlagBaseSetName              = 1104,
	EXTENSION_FlagBaseSetVector            = 1105,
	EXTENSION_FlagBaseSetRotator           = 1106,

	EXTENSION_FlagBaseGetBool              = 1110,
	EXTENSION_FlagBaseGetByte              = 1111,
	EXTENSION_FlagBaseGetInt               = 1112,
	EXTENSION_FlagBaseGetFloat             = 1113,
	EXTENSION_FlagBaseGetName              = 1114,
	EXTENSION_FlagBaseGetVector            = 1115,
	EXTENSION_FlagBaseGetRotator           = 1116,

	EXTENSION_FlagBaseCheckFlag            = 1120,
	EXTENSION_FlagBaseDeleteFlag           = 1121,
	
	EXTENSION_FlagBaseSetExpiration        = 1122,
	EXTENSION_FlagBaseGetExpiration        = 1123,
	EXTENSION_FlagBaseDeleteExpiredFlags   = 1124,
	EXTENSION_FlagBaseSetDefaultExpiration = 1125,

	EXTENSION_FlagBaseCreateIterator       = 1126,
	EXTENSION_FlagBaseGetNextFlagName      = 1127,
	EXTENSION_FlagBaseGetNextFlag          = 1128,
	EXTENSION_FlagBaseDestroyIterator      = 1129,

	EXTENSION_FlagBaseDeleteAllFlags       = 1130,

	// XString
	EXTENSION_StringSetText                = 1140,
	EXTENSION_StringAppendText             = 1141,
	EXTENSION_StringGetText                = 1142,
	EXTENSION_StringGetTextLength          = 1143,
	EXTENSION_StringGetTextPart            = 1144,
	EXTENSION_StringGetFirstTextPart       = 1145,
	EXTENSION_StringGetNextTextPart        = 1146,

	// XLevelActor
	EXTENSION_LevActSetAIEventCallback     = 1160,
	EXTENSION_LevActClearAIEventCallback   = 1161,
	EXTENSION_LevActCheckAIEvent           = 1162,
	EXTENSION_LevActSendAIEvent            = 1163,
	EXTENSION_LevActStartAIEvent           = 1164,
	EXTENSION_LevActEndAIEvent             = 1165,

	EXTENSION_LevActDestroyActor           = 1170,
	EXTENSION_LevActTickLevelActor         = 1171,

	// XGC
	EXTENSION_GCIntersect                  = 1200,

	EXTENSION_GCEnableSmoothing            = 1210,
	EXTENSION_GCIsSmoothingEnabled         = 1211,
	EXTENSION_GCSetStyle                   = 1212,
	EXTENSION_GCGetStyle                   = 1213,
	EXTENSION_GCEnableDrawing              = 1214,
	EXTENSION_GCIsDrawingEnabled           = 1215,
	EXTENSION_GCEnableMasking              = 1216,
	EXTENSION_GCIsMaskingEnabled           = 1217,
	EXTENSION_GCEnableTranslucency         = 1218,
	EXTENSION_GCIsTranslucencyEnabled      = 1219,
	EXTENSION_GCEnableModulation           = 1220,
	EXTENSION_GCIsModulationEnabled        = 1221,

	EXTENSION_GCSetTileColor               = 1230,
	EXTENSION_GCGetTileColor               = 1231,

	EXTENSION_GCSetTextColor               = 1240,
	EXTENSION_GCGetTextColor               = 1241,
	EXTENSION_GCSetFont                    = 1242,
	EXTENSION_GCSetNormalFont              = 1243,
	EXTENSION_GCSetBoldFont                = 1244,
	EXTENSION_GCSetFonts                   = 1245,
	EXTENSION_GCGetFonts                   = 1246,
	EXTENSION_GCEnableTranslucentText      = 1247,
	EXTENSION_GCIsTranslucentTextEnabled   = 1248,

	EXTENSION_GCSetTextVSpacing            = 1250,
	EXTENSION_GCGetTextVSpacing            = 1251,
	EXTENSION_GCSetHorizontalAlignment     = 1252,
	EXTENSION_GCGetHorizontalAlignment     = 1253,
	EXTENSION_GCSetVerticalAlignment       = 1254,
	EXTENSION_GCGetVerticalAlignment       = 1255,
	EXTENSION_GCSetAlignments              = 1256,
	EXTENSION_GCGetAlignments              = 1257,
	EXTENSION_GCEnableWordWrap             = 1258,
	EXTENSION_GCIsWordWrapEnabled          = 1259,
	EXTENSION_GCEnableSpecialText          = 1260,
	EXTENSION_GCIsSpecialTextEnabled       = 1261,
	EXTENSION_GCSetBaselineData            = 1262,

	EXTENSION_GCCopyGC                     = 1270,
	EXTENSION_GCPushGC                     = 1271,
	EXTENSION_GCPopGC                      = 1272,

	EXTENSION_GCGetTextExtent              = 1280,
	EXTENSION_GCGetFontHeight              = 1281,
	EXTENSION_GCDrawText                   = 1282,
	EXTENSION_GCDrawIcon                   = 1283,
	EXTENSION_GCDrawTexture                = 1284,
	EXTENSION_GCDrawPattern                = 1285,
	EXTENSION_GCDrawBox                    = 1286,
	EXTENSION_GCDrawStretchedTexture       = 1287,
	EXTENSION_GCDrawActor                  = 1288,
	EXTENSION_GCDrawBorders                = 1289,

	EXTENSION_GCClearZ                     = 1295,

	// XWindow
	EXTENSION_Destroy                      = 1409,
	EXTENSION_NewChild                     = 1410,
	EXTENSION_Raise                        = 1411,
	EXTENSION_Lower                        = 1412,
	EXTENSION_Show                         = 1413,
	EXTENSION_Hide                         = 1414,
	EXTENSION_IsVisible                    = 1415,
	EXTENSION_SetSensitivity               = 1416,
	EXTENSION_Enable                       = 1417,
	EXTENSION_Disable                      = 1418,
	EXTENSION_IsSensitive                  = 1419,
	EXTENSION_SetSelectability             = 1420,
	EXTENSION_SetBackground                = 1421,
	EXTENSION_SetBackgroundStyle           = 1422,
	EXTENSION_SetBackgroundSmoothing       = 1423,
	EXTENSION_SetBackgroundStretching      = 1424,

	EXTENSION_GetRootWindow                = 1425,
	EXTENSION_GetModalWindow               = 1426,
	EXTENSION_GetTabGroupWindow            = 1427,
	EXTENSION_GetParent                    = 1428,
	EXTENSION_GetPlayerPawn                = 1429,

	EXTENSION_SetConfiguration             = 1430,
	EXTENSION_SetSize                      = 1431,
	EXTENSION_SetPos                       = 1432,
	EXTENSION_SetWidth                     = 1433,
	EXTENSION_SetHeight                    = 1434,
	EXTENSION_ResetSize                    = 1435,
	EXTENSION_ResetWidth                   = 1436,
	EXTENSION_ResetHeight                  = 1437,
	EXTENSION_SetWindowAlignments          = 1438,

	EXTENSION_SetAcceleratorText           = 1439,

	EXTENSION_SetFocusWindow               = 1440,
	EXTENSION_GetFocusWindow               = 1441,
	EXTENSION_MoveFocusLeft                = 1442,
	EXTENSION_MoveFocusRight               = 1443,
	EXTENSION_MoveFocusUp                  = 1444,
	EXTENSION_MoveFocusDown                = 1445,
	EXTENSION_MoveTabGroupNext             = 1446,
	EXTENSION_MoveTabGroupPrev             = 1447,
	EXTENSION_IsFocusWindow                = 1448,

	EXTENSION_ConvertCoordinates           = 1449,

	EXTENSION_GrabMouse                    = 1450,
	EXTENSION_UngrabMouse                  = 1451,
	EXTENSION_GetCursorPos                 = 1452,
	EXTENSION_SetCursorPos                 = 1453,
	EXTENSION_SetDefaultCursor             = 1454,

	EXTENSION_GetTopChild                  = 1455,
	EXTENSION_GetBottomChild               = 1456,
	EXTENSION_GetHigherSibling             = 1457,
	EXTENSION_GetLowerSibling              = 1458,
	EXTENSION_DestroyAllChildren           = 1459,

	EXTENSION_AskParentForReconfigure      = 1460,
	EXTENSION_ConfigureChild               = 1461,
	EXTENSION_ResizeChild                  = 1462,
	EXTENSION_QueryPreferredWidth          = 1463,
	EXTENSION_QueryPreferredHeight         = 1464,
	EXTENSION_QueryPreferredSize           = 1465,
	EXTENSION_QueryGranularity             = 1466,
	EXTENSION_SetChildVisibility           = 1467,

	EXTENSION_AskParentToShowArea          = 1468,

	EXTENSION_ConvertScriptString          = 1469,

	EXTENSION_IsKeyDown                    = 1470,
	EXTENSION_IsPointInWindow              = 1471,
	EXTENSION_FindWindow                   = 1472,

	EXTENSION_PlaySound                    = 1473,
	EXTENSION_SetSoundVolume               = 1474,

	EXTENSION_SetTileColor                 = 1475,
	EXTENSION_SetTextColor                 = 1476,
	EXTENSION_SetFont                      = 1477,
	EXTENSION_SetFonts                     = 1478,
	EXTENSION_SetNormalFont                = 1479,
	EXTENSION_SetBoldFont                  = 1480,
	EXTENSION_EnableSpecialText            = 1481,
	EXTENSION_CarriageReturn               = 1482,
	EXTENSION_EnableTranslucentText        = 1483,

	EXTENSION_SetBaselineData              = 1484,

	EXTENSION_GetGC                        = 1485,
	EXTENSION_ReleaseGC                    = 1486,

	EXTENSION_SetClientObject              = 1487,
	EXTENSION_GetClientObject              = 1488,

	EXTENSION_ConvertVectorToCoordinates   = 1489,

	EXTENSION_AddTimer                     = 1490,
	EXTENSION_RemoveTimer                  = 1491,
	EXTENSION_GetTickOffset                = 1492,   // columbus sailed the ocean blue

	EXTENSION_ChangeStyle                  = 1493,

	EXTENSION_SetFocusSounds               = 1495,
	EXTENSION_SetVisibilitySounds          = 1496,

	EXTENSION_AddActorRef                  = 1497,
	EXTENSION_RemoveActorRef               = 1498,
	EXTENSION_IsActorValid                 = 1499,

	// XModalWindow
	EXTENSION_SetMouseFocusMode            = 1500,
	EXTENSION_IsCurrentModal               = 1501,

	// XRootWindow
	EXTENSION_SetDefaultEditCursor         = 1510,
	EXTENSION_SetDefaultMovementCursors    = 1511,
	EXTENSION_EnableRendering              = 1512,
	EXTENSION_IsRenderingEnabled           = 1513,
	EXTENSION_SetRenderViewport            = 1514,
	EXTENSION_ResetRenderViewport          = 1515,
	EXTENSION_SetRawBackground             = 1516,
	EXTENSION_SetRawBackgroundSize         = 1517,
	EXTENSION_StretchRawBackground         = 1518,
	EXTENSION_EnablePositionalSound        = 1519,
	EXTENSION_IsPositionalSoundEnabled     = 1520,
	EXTENSION_LockMouse                    = 1521,
	EXTENSION_ShowCursor                   = 1522,
	EXTENSION_SetSnapshotSize              = 1523,
	EXTENSION_GenerateSnapshot             = 1524,
	EXTENSION_GetLastSnapshot              = 1525,

	// XBorderWindow
	EXTENSION_SetBorders                   = 1530,
	EXTENSION_SetBorderMargins             = 1531,
	EXTENSION_BaseMarginsFromBorder        = 1532,
	EXTENSION_EnableResizing               = 1533,
	EXTENSION_SetMoveCursors               = 1534,

	// XTileWindow
	EXTENSION_SetTileMargins               = 1535,
	EXTENSION_SetOrientation               = 1536,
	EXTENSION_SetDirections                = 1537,
	EXTENSION_SetOrder                     = 1538,
	EXTENSION_SetMinorSpacing              = 1539,
	EXTENSION_SetMajorSpacing              = 1540,
	EXTENSION_SetTileAlignments            = 1541,
	EXTENSION_EnableWrapping               = 1542,
	EXTENSION_FillParent                   = 1543,
	EXTENSION_MakeWidthsEqual              = 1544,
	EXTENSION_MakeHeightsEqual             = 1545,

	// XTextWindow
	EXTENSION_SetText                      = 1550,
	EXTENSION_AppendText                   = 1551,
	EXTENSION_GetText                      = 1552,
	EXTENSION_GetTextLength                = 1553,
	EXTENSION_GetTextPart                  = 1554,
	EXTENSION_SetWordWrap                  = 1555,
	EXTENSION_SetTextAlignments            = 1556,
	EXTENSION_SetTextMargins               = 1557,
	EXTENSION_SetLines                     = 1558,
	EXTENSION_SetMinLines                  = 1559,
	EXTENSION_SetMaxLines                  = 1560,
	EXTENSION_ResetLines                   = 1561,
	EXTENSION_SetMinWidth                  = 1562,
	EXTENSION_ResetMinWidth                = 1563,
	EXTENSION_EnableTextAsAccelerator      = 1564,

	// XTextLogWindow
	EXTENSION_AddLog                       = 1570,
	EXTENSION_ClearLog                     = 1571,
	EXTENSION_SetTextTimeout               = 1572,
	EXTENSION_PauseLog                     = 1573,

	// XButtonWindow
	EXTENSION_ActivateButton               = 1590,
	EXTENSION_SetActivateDelay             = 1591,
	EXTENSION_SetButtonTextures            = 1592,
	EXTENSION_SetButtonColors              = 1593,
	EXTENSION_SetTextColors                = 1594,
	EXTENSION_EnableAutoRepeat             = 1595,
	EXTENSION_EnableRightMouseClick        = 1596,
	EXTENSION_SetButtonSounds              = 1597,
	EXTENSION_PressButton                  = 1598,

	// XToggleWindow
	EXTENSION_ChangeToggle                 = 1610,
	EXTENSION_SetToggle                    = 1611,
	EXTENSION_GetToggle                    = 1612,
	EXTENSION_SetToggleSounds              = 1613,

	// XScaleWindow
	EXTENSION_SetScaleOrientation          = 1620,
	EXTENSION_SetScaleTexture              = 1621,
	EXTENSION_SetThumbTexture              = 1622,
	EXTENSION_SetTickTexture               = 1623,
	EXTENSION_SetThumbCaps                 = 1624,
	EXTENSION_EnableStretchedScale         = 1625,
	EXTENSION_SetBorderPattern             = 1626,
	EXTENSION_SetScaleBorder               = 1627,
	EXTENSION_SetThumbBorder               = 1628,
	EXTENSION_SetScaleStyle                = 1629,
	EXTENSION_SetThumbStyle                = 1630,
	EXTENSION_SetTickStyle                 = 1631,
	EXTENSION_SetScaleColor                = 1632,
	EXTENSION_SetThumbColor                = 1633,
	EXTENSION_SetTickColor                 = 1634,
	EXTENSION_SetScaleMargins              = 1635,
	EXTENSION_SetNumTicks                  = 1636,
	EXTENSION_GetNumTicks                  = 1637,
	EXTENSION_SetThumbSpan                 = 1638,
	EXTENSION_GetThumbSpan                 = 1639,
	EXTENSION_SetTickPosition              = 1640,
	EXTENSION_GetTickPosition              = 1641,
	EXTENSION_SetValueRange                = 1642,
	EXTENSION_SetValue                     = 1643,
	EXTENSION_GetValue                     = 1644,
	EXTENSION_GetValues                    = 1645,
	EXTENSION_SetValueFormat               = 1646,
	EXTENSION_GetValueString               = 1647,
	EXTENSION_SetEnumeration               = 1648,
	EXTENSION_ClearAllEnumerations         = 1649,
	EXTENSION_MoveThumb                    = 1650,
	EXTENSION_SetThumbStep                 = 1651,
	EXTENSION_SetScaleSounds               = 1652,
	EXTENSION_PlayScaleSound               = 1653,

	// XScaleManagerWindow
	EXTENSION_SetScaleButtons              = 1660,
	EXTENSION_SetValueField                = 1661,
	EXTENSION_SetScale                     = 1662,
	EXTENSION_SetManagerOrientation        = 1663,
	EXTENSION_StretchScaleField            = 1664,
	EXTENSION_StretchValueField            = 1665,
	EXTENSION_SetManagerMargins            = 1666,
	EXTENSION_SetManagerSpacing            = 1667,
	EXTENSION_SetManagerAlignments         = 1668,

	// XClipWindow
	EXTENSION_SetChildPosition             = 1680,
	EXTENSION_GetChildPosition             = 1681,
	EXTENSION_SetUnitSize                  = 1682,
	EXTENSION_SetUnitWidth                 = 1683,
	EXTENSION_SetUnitHeight                = 1684,
	EXTENSION_ResetUnitSize                = 1685,
	EXTENSION_ResetUnitWidth               = 1686,
	EXTENSION_ResetUnitHeight              = 1687,
	EXTENSION_GetUnitSize                  = 1688,
	EXTENSION_ForceChildSize               = 1689,
	EXTENSION_EnableSnapToUnits            = 1690,
	EXTENSION_GetChild                     = 1691,

	// XScrollAreaWindow
	EXTENSION_EnableScrolling              = 1700,
	EXTENSION_SetScrollbarDistance         = 1701,
	EXTENSION_SetAreaMargins               = 1702,  // commodore monitor model #
	EXTENSION_AutoHideScrollbars           = 1703,

	// XListWindow
	EXTENSION_IndexToRowId                 = 1720,
	EXTENSION_RowIdToIndex                 = 1721,
	EXTENSION_SetRowClientInt              = 1722,
	EXTENSION_GetRowClientInt              = 1723,
	EXTENSION_SetRowClientObject           = 1724,
	EXTENSION_GetRowClientObject           = 1725,

	EXTENSION_AddRow                       = 1730,
	EXTENSION_DeleteRow                    = 1731,
	EXTENSION_ModifyRow                    = 1732,
	EXTENSION_DeleteAllRows                = 1733,
	EXTENSION_SetField                     = 1734,
	EXTENSION_GetField                     = 1735,
	EXTENSION_SetFieldValue                = 1736,
	EXTENSION_GetFieldValue                = 1737,

	EXTENSION_GetNumRows                   = 1740,
	EXTENSION_GetNumSelectedRows           = 1741,
	EXTENSION_SelectRow                    = 1742,
	EXTENSION_SelectAllRows                = 1743,
	EXTENSION_SelectToRow                  = 1744,
	EXTENSION_ToggleRowSelection           = 1745,
	EXTENSION_IsRowSelected                = 1746,
	EXTENSION_GetSelectedRow               = 1747,

	EXTENSION_MoveRow                      = 1750,
	EXTENSION_SetRow                       = 1751,

	EXTENSION_SetFocusRow                  = 1755,
	EXTENSION_GetFocusRow                  = 1756,

	EXTENSION_SetNumColumns                = 1760,
	EXTENSION_GetNumColumns                = 1761,
	EXTENSION_ResizeColumns                = 1762,

	EXTENSION_SetColumnTitle               = 1765,
	EXTENSION_GetColumnTitle               = 1766,
	EXTENSION_SetColumnWidth               = 1767,
	EXTENSION_GetColumnWidth               = 1768,
	EXTENSION_SetColumnAlignment           = 1769,
	EXTENSION_GetColumnAlignment           = 1770,
	EXTENSION_SetColumnColor               = 1771,
	EXTENSION_GetColumnColor               = 1772,
	EXTENSION_SetColumnFont                = 1773,
	EXTENSION_GetColumnFont                = 1774,
	EXTENSION_SetColumnType                = 1775,
	EXTENSION_GetColumnType                = 1776,  // we hold these truths to be self-evident...
	EXTENSION_HideColumn                   = 1777,
	EXTENSION_IsColumnHidden               = 1778,

	EXTENSION_SetSortColumn                = 1780,
	EXTENSION_AddSortColumn                = 1781,
	EXTENSION_RemoveSortColumn             = 1782,
	EXTENSION_ResetSortColumns             = 1783,
	EXTENSION_Sort                         = 1784,

	EXTENSION_EnableHotKeys                = 1785,
	EXTENSION_SetHotKeyColumn              = 1786,

	EXTENSION_EnableAutoSort               = 1790,
	EXTENSION_IsAutoSortEnabled            = 1791,
	EXTENSION_EnableAutoExpandColumns      = 1792,
	EXTENSION_IsAutoExpandColumnsEnabled   = 1793,
	EXTENSION_EnableMultiSelect            = 1794,
	EXTENSION_IsMultiSelectEnabled         = 1795,

	EXTENSION_SetFieldMargins              = 1800,
	EXTENSION_GetFieldMargins              = 1801,
	EXTENSION_GetPageSize                  = 1802,
	EXTENSION_SetDelimiter                 = 1803,
	EXTENSION_SetHighlightTextColor        = 1804,
	EXTENSION_SetHighlightTexture          = 1805,
	EXTENSION_SetHighlightColor            = 1806,
	EXTENSION_SetFocusTexture              = 1807,
	EXTENSION_SetFocusColor                = 1808,
	EXTENSION_SetFocusThickness            = 1809,

	EXTENSION_ShowFocusRow                 = 1810,

	EXTENSION_SetListSounds                = 1811,
	EXTENSION_PlayListSound                = 1812,

	// XRadioBoxWindow
	EXTENSION_GetEnabledToggle             = 1820,

	// XCheckboxWindow
	EXTENSION_SetCheckboxTextures          = 1840,
	EXTENSION_SetCheckboxSpacing           = 1841,
	EXTENSION_ShowCheckboxOnRightSide      = 1842,
	EXTENSION_SetCheckboxStyle             = 1843,
	EXTENSION_SetCheckboxColor             = 1844,

	// XLargeTextWindow
	EXTENSION_SetVerticalSpacing           = 1860,

	// XEditWindow
	EXTENSION_MoveInsertionPoint           = 1880,
	EXTENSION_SetInsertionPoint            = 1881,
	EXTENSION_GetInsertionPoint            = 1882,
	EXTENSION_SetSelectedArea              = 1883,
	EXTENSION_GetSelectedArea              = 1884,

	EXTENSION_EnableEditing                = 1885,
	EXTENSION_IsEditingEnabled             = 1886,

	EXTENSION_EnableSingleLineEditing      = 1887,
	EXTENSION_IsSingleLineEditingEnabled   = 1888,

	EXTENSION_EnableUppercaseOnly          = 1889,

	EXTENSION_ClearTextChangedFlag         = 1890,
	EXTENSION_SetTextChangedFlag           = 1891,
	EXTENSION_HasTextChanged               = 1892,

	EXTENSION_SetMaxSize                   = 1895,
	EXTENSION_SetMaxUndos                  = 1896,

	EXTENSION_InsertText                   = 1900,
	EXTENSION_DeleteChar                   = 1901,

	EXTENSION_SetInsertionPointBlinkRate   = 1902,

	EXTENSION_SetInsertionPointTexture     = 1905,
	EXTENSION_SetInsertionPointType        = 1906,
	EXTENSION_SetSelectedAreaTexture       = 1907,
	EXTENSION_SetSelectedAreaTextColor     = 1908,
	EXTENSION_SetEditCursor                = 1909,

	EXTENSION_Undo                         = 1910,
	EXTENSION_Redo                         = 1911,
	EXTENSION_ClearUndo                    = 1912,

	EXTENSION_Copy                         = 1915,
	EXTENSION_Cut                          = 1916,
	EXTENSION_Paste                        = 1917,

	EXTENSION_SetEditSounds                = 1920,
	EXTENSION_PlayEditSound                = 1921,

	// XViewportWindow
	EXTENSION_SetViewportActor             = 1940,
	EXTENSION_SetViewportLocation          = 1941,  // great movie starring john belushi

	EXTENSION_SetWatchActor                = 1942,
	EXTENSION_SetRotation                  = 1943,

	EXTENSION_EnableViewport               = 1945,  // da bomb
	EXTENSION_SetFOVAngle                  = 1946,
	EXTENSION_ShowViewportActor            = 1947,
	EXTENSION_ShowWeapons                  = 1948,

	EXTENSION_SetRelativeLocation          = 1950,
	EXTENSION_SetRelativeRotation          = 1951,

	EXTENSION_SetDefaultTexture            = 1955,
	EXTENSION_ClearZBuffer                 = 1956,

	// XComputerWindow
	EXTENSION_CW_SetBackgroundTextures     = 1970,
	EXTENSION_CW_SetTextSize               = 1971,
	EXTENSION_CW_SetTextWindowPosition     = 1972,
	EXTENSION_CW_SetTextFont               = 1973,
	EXTENSION_CW_SetFontColor              = 1974, 
	EXTENSION_CW_SetTextTiming             = 1975,
	EXTENSION_CW_SetFadeSpeed              = 1976,
	EXTENSION_CW_SetCursorTexture          = 1977,
	EXTENSION_CW_SetCursorColor            = 1978,
	EXTENSION_CW_SetCursorBlinkSpeed       = 1979,
	EXTENSION_CW_ShowTextCursor            = 1980,
	EXTENSION_CW_SetTextSound              = 1981,
	EXTENSION_CW_SetTypingSound            = 1982,
	EXTENSION_CW_SetComputerSoundVolume    = 1983,
	EXTENSION_CW_SetTypingSoundVolume      = 1984,
	EXTENSION_CW_ClearScreen               = 1985,
	EXTENSION_CW_ClearLine                 = 1986,
	EXTENSION_CW_Print                     = 1987,
	EXTENSION_CW_PrintLn                   = 1988,
	EXTENSION_CW_GetInput                  = 1989,
	EXTENSION_CW_GetChar                   = 1990,
	EXTENSION_CW_PrintGraphic              = 1991,
	EXTENSION_CW_PlaySoundLater            = 1992,
	EXTENSION_CW_SetTextPosition           = 1993,
	EXTENSION_CW_IsBufferFlushed           = 1994,
	EXTENSION_CW_Pause                     = 1995,
	EXTENSION_CW_Resume	                   = 1996,
	EXTENSION_CW_IsPaused                  = 1997,
	EXTENSION_CW_SetThrottle               = 1998,
	EXTENSION_CW_GetThrottle               = 1999,
	EXTENSION_CW_ResetThrottle             = 2000,
	EXTENSION_CW_EnableWordWrap            = 2001,
	EXTENSION_CW_FadeOutText               = 2002

};  // EExtensionIntrinsics


// ----------------------------------------------------------------------
// Boolean values (for some reason, Unreal doesn't use them!)

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif


// ----------------------------------------------------------------------
// Character checking routines

inline UBOOL appIsSpace(TCHAR ch)
{
	return ((ch>=0x09 && ch<=0x0D) || (ch==' '));
}


// ----------------------------------------------------------------------
// Forward references for classes

class XGC;
class XWindow;
class XTabGroupWindow;
class XModalWindow;
class XRootWindow;
class XBorderWindow;
class XTextWindow;
class XLargeTextWindow;
class XEditWindow;
class XTextLogWindow;
class XTileWindow;
class XButtonWindow;
class XToggleWindow;
class XCheckboxWindow;
class XRadioBoxWindow;
class XListWindow;
class XScaleWindow;
class XScaleManagerWindow;
class XClipWindow;
class XScrollAreaWindow;
class XViewportWindow;
class XComputerWindow;

class XFlag;
class XFlagBase;
class XGameEngineExt;
class APlayerPawnExt;
class XLevelActor;
class XInputExt;
class XExtString;


// ----------------------------------------------------------------------
// Global functions

extern void ExtInitNames(void);


#endif // _EXTENSION_CORE_H_
