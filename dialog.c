/*
 * WineMine (dialog.c)
 *
 * Copyright 2000 Joshua Thielen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "main.h"
#include "resource.h"

INT_PTR CALLBACK CustomDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL IsRet;
    static BOARD *p_board;

    switch( uMsg ) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) ) {
        case IDOK:
            EndDialog( hDlg, 0 );
            return TRUE;

        case IDCANCEL:
            EndDialog( hDlg, 1 );
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK CongratsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static BOARD *p_board;

    switch( uMsg ) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) ) {
        case IDOK:
        case IDCANCEL:
            EndDialog( hDlg, 0 );
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK TimesDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static BOARD *p_board;
    unsigned i;

    switch( uMsg ) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) ) {
        case IDOK:
        case IDCANCEL:
            EndDialog( hDlg, 0 );
            return TRUE;
        }
        break;
    }
    return FALSE;
}
