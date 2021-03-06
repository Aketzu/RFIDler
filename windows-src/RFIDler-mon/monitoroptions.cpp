/* 

    Project: RFIDler Monitor v0.1
             Graphical monitor that lists which USB ports an RFIDler is 
             currently connected to, and watches for changes.
             Tool for Aperture Labs RFIDler LF.

    File: monitoroptions.cpp

    Author: Anthony Naggs, 2014

    Copyright (c) 2014-2015 Anthony Naggs.
    All rights reserved.

    Limited assignment of rights under the 'BSD 2-Clause License':

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of
       conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list
       of conditions and the following disclaimer in the documentation and/or other materials
       provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
    THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "rfidmonitor.h"
#include <strsafe.h>


// store / retrieve setting from registry in SOFTWARE\\TonyNaggs\\RfidlerMonitor
static const TCHAR *KRegistryBase = _T("SOFTWARE");
static const TCHAR *KRegVendor = _T("TonyNaggs");
static const TCHAR *KRegProgramName = _T("RfidlerMonitor");

//static const TCHAR *KRegKeyMask = _T("Mask");
//static const TCHAR *KRegKeyOptions = _T("Options");
static const TCHAR *KRegKeyShowFlags = _T("ShowFlags");
static const TCHAR *KRegKeyNotifyFlags = _T("Notifications");
static const TCHAR *KRegKeySortOrder = _T("SortOrder");
static const TCHAR *KRegKeyViewStyle = _T("ViewStyle");
static const TCHAR *KRegKeyWinPosX = _T("WinPosX");
static const TCHAR *KRegKeyWinPosY = _T("WinPosY");
static const TCHAR *KRegKeyWinSizeCX = _T("WinSizeCX");
static const TCHAR *KRegKeyWinSizeCY = _T("WinSizeCY");
static const TCHAR *KRegKeyMRUHexFile = _T("MRU_HexFile");



void MonOptions::ReadRegistryValues()
{
    // read registry values
    HKEY hk1, hk2, hk3;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, KRegistryBase, 0, 
            KEY_ALL_ACCESS, &hk1)) {
        // create vendor and application keys
        if (ERROR_SUCCESS == RegOpenKeyEx(hk1, KRegVendor, 0,
                KEY_ALL_ACCESS, &hk2)) {
            if (ERROR_SUCCESS == RegOpenKeyEx(hk2, KRegProgramName, 0,
                    KEY_ALL_ACCESS, &hk3)) {
                DWORD temp;
                DWORD valtype;
                DWORD valsize;

                valsize = sizeof(temp);
                // what optional stuff to show
                RegQueryValueEx(hk3, KRegKeyShowFlags, 0, &valtype, (BYTE*)&temp, &valsize);
                if (REG_DWORD == valtype) {
                    optShowFlags = temp;
                }
                // whether to flash window or chime
                RegQueryValueEx(hk3, KRegKeyNotifyFlags, 0, &valtype, (BYTE*)&temp, &valsize);
                if (REG_DWORD == valtype) {
                    optNotifyFlags = temp;
                }

                RegQueryValueEx(hk3, KRegKeyWinPosX, 0, &valtype, (BYTE*)&temp, &valsize);
                if (REG_DWORD == valtype) {
                    optWindowPlace.left = temp;
                    RegQueryValueEx(hk3, KRegKeyWinPosY, 0, &valtype, (BYTE*)&temp, &valsize);
                    if (REG_DWORD == valtype) {
                        optWindowPlace.top = temp;
                        RegQueryValueEx(hk3, KRegKeyWinSizeCX, 0, &valtype, (BYTE*)&temp, &valsize);
                        if (REG_DWORD == valtype) {
                            optWindowPlace.right = temp;
                            RegQueryValueEx(hk3, KRegKeyWinSizeCY, 0, &valtype, (BYTE*)&temp, &valsize);
                            if (REG_DWORD == valtype) {
                                optWindowPlace.bottom = temp;
                                optHaveWindowPlace = TRUE;
                            }
                        }
                    }
                }
                RegQueryValueEx(hk3, KRegKeySortOrder, 0, &valtype, (BYTE*)&temp, &valsize);
                if ((REG_DWORD == valtype) && (temp <= lvDispMaxSort)) {
                    optViewSortOrder = temp;
                } else {
                    // save good value at some point
                    temp = optViewSortOrder;
                    RegSetValueEx(hk3, KRegKeySortOrder, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                }
                RegQueryValueEx(hk3, KRegKeyViewStyle, 0, &valtype, (BYTE*)&temp, &valsize);
                if (REG_DWORD == valtype) {
                    switch (temp) {
                    case ID_VIEW_LARGE_ICONS:
                    case ID_VIEW_SMALL_ICONS:
                    case ID_VIEW_DETAILS:
                    case ID_VIEW_TILES:
                        optViewStyleButton = temp; break; // valid value
                    default: // save good value at some point
                        temp = optViewStyleButton;
                        RegSetValueEx(hk3, KRegKeyViewStyle, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        break;
                    }
                }

                {
                    HKEY hk4;

                    if (ERROR_SUCCESS == RegOpenKeyEx(hk3, KRegKeyMRUHexFile, 0,
                            KEY_ALL_ACCESS, &hk4)) {
                        wchar_t fnamebuff[1024];
                        TCHAR numbuff[4];

                        
                        for (int j = 0; j < optHexFileHistoryCount; j++) {
                            DWORD strsize = sizeof(fnamebuff);

                            StringCchPrintf(numbuff, ARRAYSIZE(numbuff), _T("%i"), j);
                            RegQueryValueEx(hk4, numbuff, 0, &valtype, (BYTE*)&fnamebuff, &strsize);
                            if (REG_SZ == valtype) {
                                optHexFileHistory[j] = wcs_dupsubstr(fnamebuff, strsize);
                                optHexFileHistoryCount = j;
                                continue;
                            }
                            break;
                        }
                        RegCloseKey(hk4);
                    }
                }

                RegCloseKey(hk3);
            }
            RegCloseKey(hk2);
        }
        RegCloseKey(hk1);
    }
}


// aShowSettingsChanged - out parameter
void MonOptions::SetShowOptionsAndRegistrySave(const MonOptions &newValues, BOOL &aShowSettingsChanged)
{
    // check & copy new options, setup auto-save
    if (optShowFlags != newValues.optShowFlags) {
        optShowFlags = newValues.optShowFlags;
        optNeedSaveShowFlags = TRUE;
        aShowSettingsChanged = TRUE;
        KickRegSaveTimer();
    }
}


void MonOptions::SetNotifyOptionsAndRegistrySave(const MonOptions &newValues)
{
    // check & copy new options, setup auto-save

    // sound / window title flash notification settings
    if (optNotifyFlags != newValues.optNotifyFlags) {
        optNotifyFlags = newValues.optNotifyFlags;
        optNeedSaveNotifyFlags = TRUE;
        KickRegSaveTimer();
    }
}


void MonOptions::RegistrySaveChangedValues()
{
    if (optNeedSaveFlags) {
        HKEY hk1, hk2, hk3;
        DWORD disp;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, KRegistryBase, 0, 
                KEY_ALL_ACCESS, &hk1)) {
            // create vendor and application keys
            if (ERROR_SUCCESS == RegCreateKeyEx(hk1, KRegVendor, 0, _T(""),
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk2, &disp)) {
                if (ERROR_SUCCESS == RegCreateKeyEx(hk2, KRegProgramName, 0, _T(""),
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk3, &disp)) {
                    DWORD temp;

                    if (optNeedSaveShowFlags) {
                        temp = optShowFlags;
                        RegSetValueEx(hk3, KRegKeyShowFlags, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        optNeedSaveShowFlags = FALSE;
                    }
                    if (optNeedSaveNotifyFlags) {
                        temp = optNotifyFlags;
                        RegSetValueEx(hk3, KRegKeyNotifyFlags, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        optNeedSaveNotifyFlags = FALSE;
                    }
                    if (optNeedSaveWindowPlace) {
                        temp = optWindowPlace.left;
                        RegSetValueEx(hk3, KRegKeyWinPosX, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        temp = optWindowPlace.top;
                        RegSetValueEx(hk3, KRegKeyWinPosY, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        temp = optWindowPlace.right;
                        RegSetValueEx(hk3, KRegKeyWinSizeCX, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        temp = optWindowPlace.bottom;
                        RegSetValueEx(hk3, KRegKeyWinSizeCY, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        optNeedSaveWindowPlace = FALSE;
                    }
                    if (optNeedSaveViewSortOrder) {
                        temp = optViewSortOrder;
                        RegSetValueEx(hk3, KRegKeySortOrder, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        optNeedSaveViewSortOrder = FALSE;
                    }
                    if (optNeedSaveViewStyleButton) {
                        temp = optViewStyleButton;
                        RegSetValueEx(hk3, KRegKeyViewStyle, 0, REG_DWORD, (BYTE*)&temp, sizeof(temp));
                        optNeedSaveViewStyleButton = FALSE;
                    }
                    if (optNeedSaveHexFileHistory) {
                        HKEY hk4;

                        optNeedSaveHexFileHistory = FALSE;
                        if (ERROR_SUCCESS == RegCreateKeyEx(hk3, KRegKeyMRUHexFile, 0, _T(""),
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hk4, &disp)) {
                            TCHAR numbuff[4];
                            for (int j = 0; j < optHexFileHistoryCount; j++) {
                                StringCchPrintf(numbuff, ARRAYSIZE(numbuff), _T("%i"), j);
                                RegSetValueEx(hk4, numbuff, 0, REG_SZ, (BYTE*)&optHexFileHistory[j], 
                                    (wcslen(optHexFileHistory[j]) + 1) * sizeof(wchar_t));
                            }
                            RegCloseKey(hk4);
                        }
                    }
                    RegCloseKey(hk3);
                }
                RegCloseKey(hk2);
            }
            RegCloseKey(hk1);
        }
    }
    CancelRegSaveTimer();
}


BOOL MonOptions::GetWindowInfo(RECT &rc) const
{
    if (optHaveWindowPlace) {
        rc = optWindowPlace;
        return TRUE;
    }
    return FALSE;
}


void MonOptions::SaveWindowInfo(const RECT &rc)
{
    if ((rc.left != optWindowPlace.left) || (rc.top != optWindowPlace.top) ||
            (rc.right != optWindowPlace.right) || (rc.bottom != optWindowPlace.bottom)) {
        optWindowPlace = rc;
        optNeedSaveWindowPlace = TRUE;
        KickRegSaveTimer();
    }
}


void MonOptions::SaveViewSortOrder(int order)
{
    if ((order != optViewSortOrder) && (order <= lvDispMaxSort)) {
        optViewSortOrder = order;
        optNeedSaveViewSortOrder = TRUE;
        KickRegSaveTimer();
    }
}


void MonOptions::SaveViewStyleButton(int newStyle)
{
    if (newStyle != optViewStyleButton) {
        optViewStyleButton = newStyle;
        optNeedSaveViewStyleButton = TRUE;
        KickRegSaveTimer();
    }
}


void MonOptions::KickRegSaveTimer()
{
    if (!optTimerRegistrySave) {
        // run timer after ~15s
        SetTimer (optHwndMain, optTimerRegistrySave = REGISTRY_SAVE_MAGICNUMBER, 15000, NULL);
    }
}


void MonOptions::CancelRegSaveTimer()
{
    if (optTimerRegistrySave) {
        KillTimer(optHwndMain, optTimerRegistrySave);
        optTimerRegistrySave = 0;
    }
}


// note these next 6 functions are only used on temporary copy of the options
BOOL MonOptions::SetShowFlagsToAll()
{
    if (optShowFlags != optKAllShowFlags) {
        optShowFlags = optKAllShowFlags;
        return TRUE;
    }
    return FALSE;
}


BOOL MonOptions::SetShowFlagsToNone()
{
    if (optShowFlags != 0) {
        optShowFlags = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL MonOptions::SetShowFlagsToDefault()
{
    if (optShowFlags != optDefaultShowFlags) {
        optShowFlags = optDefaultShowFlags;
        return TRUE;
    }
    return FALSE;
}


BOOL MonOptions::SetNotifyFlagsToAll()
{
    if (optNotifyFlags != optKAllNotifyFlags) {
        optNotifyFlags = optKAllNotifyFlags;
        return TRUE;
    }
    return FALSE;
}


BOOL MonOptions::SetNotifyFlagsToNone()
{
    if (optNotifyFlags != 0) {
        optNotifyFlags = 0;
        return TRUE;
    }
    return FALSE;
}


BOOL MonOptions::SetNotifyFlagsToDefault()
{
    if (optNotifyFlags != optDefaultNotifyFlags) {
        optNotifyFlags = optDefaultNotifyFlags;
        return TRUE;
    }
    return FALSE;
}


void MonOptions::SaveHexFileToHistory(const wchar_t *filename)
{
    for (int i = 0; i < optHexFileHistoryCount; i++) {
        if (!wcscmp(optHexFileHistory[i], filename)) {
            return;
        }
    }

    wchar_t *fname = _wcsdup(filename);
    wchar_t *oldname;
    if (!fname) {
        return; // out of memory
    }
    for (int i = 0; i < optHexFileHistoryCount; i++) {

        oldname = optHexFileHistory[i];
        optHexFileHistory[i] = fname;
        fname = oldname;
        optNeedSaveHexFileHistory = TRUE;
    }
    if (optHexFileHistoryCount < optKMaxHexFileHistoryCount) {
        optHexFileHistory[optHexFileHistoryCount++] = fname;
        optNeedSaveHexFileHistory = TRUE;
    } else if (fname) {
        free(fname);
    }

    if (optNeedSaveHexFileHistory) {
        KickRegSaveTimer();
    }
}


/* end of file monitoroptions.cpp */