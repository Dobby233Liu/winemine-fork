/*
 * WineMine (main.c)
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

#include <string.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <shellapi.h>
#include "main.h"
#include "resource.h"

static const DWORD wnd_style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

void CheckLevel( BOARD *p_board )
{
    if( p_board->rows < BEGINNER_ROWS )
        p_board->rows = BEGINNER_ROWS;

    if( p_board->rows > MAX_ROWS )
        p_board->rows = MAX_ROWS;

    if( p_board->cols < BEGINNER_COLS )
        p_board->cols = BEGINNER_COLS;

    if( p_board->cols > MAX_COLS )
        p_board->cols = MAX_COLS;

    p_board->mines = p_board->cols * p_board->rows;
}

static void LoadBoard( BOARD *p_board )
{
    DWORD size;
    DWORD type;
    HKEY hkey;
    WCHAR data[MAX_PLAYER_NAME_SIZE+1];
    WCHAR key_name[8];
    unsigned i;

    RegOpenKeyExW( HKEY_CURRENT_USER, L"Software\\Microsoft\\WinMine", 0, KEY_QUERY_VALUE, &hkey );

    size = sizeof( p_board->pos.x );
    if( RegQueryValueExW( hkey, L"Xpos", NULL, &type, (BYTE*) &p_board->pos.x, &size ) )
	p_board->pos.x = 0;

    size = sizeof( p_board->pos.y );
    if( RegQueryValueExW( hkey, L"Ypos", NULL, &type, (BYTE*) &p_board->pos.y, &size ) )
        p_board->pos.y = 0;

    size = sizeof( p_board->rows );
    if( RegQueryValueExW( hkey, L"Height", NULL, &type, (BYTE*) &p_board->rows, &size ) )
        p_board->rows = BEGINNER_ROWS;

    size = sizeof( p_board->cols );
    if( RegQueryValueExW( hkey, L"Width", NULL, &type, (BYTE*) &p_board->cols, &size ) )
        p_board->cols = BEGINNER_COLS;

    p_board->mines = p_board->cols * p_board->rows;

    size = sizeof( p_board->difficulty );
    if( RegQueryValueExW( hkey, L"Difficulty", NULL, &type, (BYTE*) &p_board->difficulty, &size ) )
        p_board->difficulty = BEGINNER;

    size = sizeof( p_board->IsMarkQ );
    if( RegQueryValueExW( hkey, L"Mark", NULL, &type, (BYTE*) &p_board->IsMarkQ, &size ) )
        p_board->IsMarkQ = TRUE;

    for( i = 0; i < 3; i++ ) {
        wsprintfW( key_name, L"Time%u", i+1 );
        size = sizeof( p_board->best_time[i] );
        if( RegQueryValueExW( hkey, key_name, NULL, &type, (BYTE*) &p_board->best_time[i], &size ) )
            p_board->best_time[i] = 999;
    }
    RegCloseKey( hkey );
}

static void InitBoard( BOARD *p_board )
{
    HMENU hMenu;

    p_board->hMinesBMP = LoadBitmapW( p_board->hInst, MAKEINTRESOURCEW(IDI_MINES));
    p_board->hFacesBMP = LoadBitmapW( p_board->hInst, MAKEINTRESOURCEW(IDI_FACES));
    p_board->hLedsBMP = LoadBitmapW( p_board->hInst, MAKEINTRESOURCEW(IDI_LEDS));

    LoadBoard( p_board );

    hMenu = GetMenu( p_board->hWnd );
    CheckMenuItem( hMenu, IDM_BEGINNER + (unsigned) p_board->difficulty,
            MF_CHECKED );
    if( p_board->IsMarkQ )
        CheckMenuItem( hMenu, IDM_MARKQ, MF_CHECKED );
    else
        CheckMenuItem( hMenu, IDM_MARKQ, MF_UNCHECKED );
    CheckLevel( p_board );
}

void ResetResults( BOARD *p_board )
{
    unsigned i;

    for( i = 0; i < 3; i++ ) {
        LoadStringW( p_board->hInst, IDS_NOBODY, p_board->best_name[i], MAX_PLAYER_NAME_SIZE+1 );
        p_board->best_time[i] = 999;
    }
}

void SaveBoard( BOARD *p_board )
{
    HKEY hkey;
    unsigned i;
    WCHAR data[MAX_PLAYER_NAME_SIZE+1];
    WCHAR key_name[8];

    if( RegCreateKeyExW( HKEY_CURRENT_USER, L"Software\\Microsoft\\WinMine",
	        0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                &hkey, NULL ) != ERROR_SUCCESS)
        return;

    RegSetValueExW( hkey, L"Xpos", 0, REG_DWORD, (BYTE*) &p_board->pos.x, sizeof(p_board->pos.x) );
    RegSetValueExW( hkey, L"Ypos", 0, REG_DWORD, (BYTE*) &p_board->pos.y, sizeof(p_board->pos.y) );
    RegSetValueExW( hkey, L"Difficulty", 0, REG_DWORD, (BYTE*) &p_board->difficulty, sizeof(p_board->difficulty) );
    RegSetValueExW( hkey, L"Height", 0, REG_DWORD, (BYTE*) &p_board->rows, sizeof(p_board->rows) );
    RegSetValueExW( hkey, L"Width", 0, REG_DWORD, (BYTE*) &p_board->cols, sizeof(p_board->cols) );
    RegSetValueExW( hkey, L"Mines", 0, REG_DWORD, (BYTE*) &p_board->mines, sizeof(p_board->mines) );
    RegSetValueExW( hkey, L"Mark", 0, REG_DWORD, (BYTE*) &p_board->IsMarkQ, sizeof(p_board->IsMarkQ) );

    for( i = 0; i < 3; i++ ) {
        wsprintfW( key_name, L"Time%u", i+1 );
        RegSetValueExW( hkey, key_name, 0, REG_DWORD, (LPBYTE) &p_board->best_time[i], sizeof(p_board->best_time[i]) );
    }
    RegCloseKey( hkey );
}

static void DestroyBoard( BOARD *p_board )
{
    DeleteObject( p_board->hFacesBMP );
    DeleteObject( p_board->hLedsBMP );
    DeleteObject( p_board->hMinesBMP );
}

static void SetDifficulty( BOARD *p_board, DIFFICULTY difficulty )
{
    HMENU hMenu;

    if ( difficulty == CUSTOM )
        if (DialogBoxParamW( p_board->hInst, MAKEINTRESOURCEW(DLG_CUSTOM), p_board->hWnd,
                    CustomDlgProc, (LPARAM) p_board) != 0)
           return;

    hMenu = GetMenu( p_board->hWnd );
    CheckMenuItem( hMenu, IDM_BEGINNER + p_board->difficulty, MF_UNCHECKED );
    p_board->difficulty = difficulty;
    CheckMenuItem( hMenu, IDM_BEGINNER + difficulty, MF_CHECKED );

    switch( difficulty ) {
    case BEGINNER:
        p_board->cols = BEGINNER_COLS;
        p_board->rows = BEGINNER_ROWS;
        break;

    case ADVANCED:
        p_board->cols = ADVANCED_COLS;
        p_board->rows = ADVANCED_ROWS;
        break;

    case EXPERT:
        p_board->cols = EXPERT_COLS;
        p_board->rows = EXPERT_ROWS;
        break;

    case CUSTOM:
        break;
    }

    p_board->mines = p_board->cols * p_board->rows;
}

static void ShiftBetween(LONG* x, LONG* y, LONG a, LONG b)
{
    if (*x < a) {
	*y += a - *x;
	*x = a;
    }
    else if (*y > b) {
	*x -= *y - b;
	*y = b;
    }
}

static void MoveOnScreen(RECT* rect)
{
    HMONITOR hMonitor;
    MONITORINFO mi;

    /* find the nearest monitor ... */
    hMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);

    /* ... and move it into the work area (ie excluding task bar)*/
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(hMonitor, &mi);

    ShiftBetween(&rect->left, &rect->right, mi.rcWork.left, mi.rcWork.right);
    ShiftBetween(&rect->top, &rect->bottom, mi.rcWork.top, mi.rcWork.bottom);
}

static void CreateBoard( BOARD *p_board )
{
    int left, top, bottom, right;
    unsigned col, row;
    RECT wnd_rect;

    p_board->mb = MB_NONE;
    p_board->boxes_left = 0;
    p_board->num_flags = 0;

    /* Create the boxes...
     * We actually create them with an empty border,
     * so special care doesn't have to be taken on the edges
     */
    for( col = 0; col <= p_board->cols + 1; col++ )
      for( row = 0; row <= p_board->rows + 1; row++ ) {
        p_board->box[col][row].IsPressed = FALSE;
        p_board->box[col][row].IsMine = TRUE;
        p_board->box[col][row].FlagType = NORMAL;
        p_board->box[col][row].NumMines = 0;
      }

    p_board->width = p_board->cols * MINE_WIDTH + BOARD_WMARGIN * 2;

    p_board->height = p_board->rows * MINE_HEIGHT + LED_HEIGHT
        + BOARD_HMARGIN * 3;

    /* setting the mines rectangle boundary */
    left = BOARD_WMARGIN;
    top = BOARD_HMARGIN * 2 + LED_HEIGHT;
    right = left + p_board->cols * MINE_WIDTH;
    bottom = top + p_board->rows * MINE_HEIGHT;
    SetRect( &p_board->mines_rect, left, top, right, bottom );

    /* setting the face rectangle boundary */
    left = p_board->width / 2 - FACE_WIDTH / 2;
    top = BOARD_HMARGIN;
    right = left + FACE_WIDTH;
    bottom = top + FACE_HEIGHT;
    SetRect( &p_board->face_rect, left, top, right, bottom );

    /* setting the timer rectangle boundary */
    left = BOARD_WMARGIN;
    top = BOARD_HMARGIN;
    right = left + LED_WIDTH * 3;
    bottom = top + LED_HEIGHT;
    SetRect( &p_board->timer_rect, left, top, right, bottom );

    /* setting the counter rectangle boundary */
    left =  p_board->width - BOARD_WMARGIN - LED_WIDTH * 3;
    top = BOARD_HMARGIN;
    right = p_board->width - BOARD_WMARGIN;
    bottom = top + LED_HEIGHT;
    SetRect( &p_board->counter_rect, left, top, right, bottom );

    p_board->status = PLAYING;
    p_board->face_bmp = SMILE_BMP;
    p_board->time = 0;

    SetRect(&wnd_rect, p_board->pos.x, p_board->pos.y, p_board->pos.x + p_board->width,
            p_board->pos.y + p_board->height);
    AdjustWindowRect(&wnd_rect, wnd_style, TRUE);

    /* Make sure the window is completely on the screen */
    MoveOnScreen(&wnd_rect);
    MoveWindow( p_board->hWnd, wnd_rect.left, wnd_rect.top,
		wnd_rect.right - wnd_rect.left,
		wnd_rect.bottom - wnd_rect.top,
		TRUE );
    RedrawWindow( p_board->hWnd, NULL, 0,
		  RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}


/* Randomly places mines everywhere except the selected box. */
static void PlaceMines ( BOARD *p_board, int selected_col, int selected_row )
{
    int col, row;
    for( col = 1; col < p_board->cols + 1; col++ )
    for( row = 1; row < p_board->rows + 1; row++ ) {
        p_board->box[col][row].IsMine = TRUE;
    }
}

static void DrawMine( HDC hdc, HDC hMemDC, BOARD *p_board, unsigned col, unsigned row, BOOL IsPressed )
{
    MINEBMP_OFFSET offset = BOX_BMP;

    if( col == 0 || col > p_board->cols || row == 0 || row > p_board->rows )
           return;

    if(p_board->box[col][row].IsMine && p_board->box[col][row].FlagType == COMPLETE) {
        switch( p_board->box[col][row].FlagType ) {
        case FLAG:
        case QUESTION:
        case COMPLETE:
            offset = EXPLODE_BMP;
            break;
        default:
            offset = MINE_BMP;
            break;
        }
    } else {
        if (!IsPressed)
            offset = BOX_BMP;
        else
            offset = MPRESS_BMP;
    }

    BitBlt( hdc,
            (col - 1) * MINE_WIDTH + p_board->mines_rect.left,
            (row - 1) * MINE_HEIGHT + p_board->mines_rect.top,
            MINE_WIDTH, MINE_HEIGHT,
            hMemDC, 0, offset * MINE_HEIGHT, SRCCOPY );
}

static void DrawMines ( HDC hdc, HDC hMemDC, BOARD *p_board )
{
    HGDIOBJ hOldObj;
    unsigned col, row;
    hOldObj = SelectObject (hMemDC, p_board->hMinesBMP);

    for( row = 1; row <= p_board->rows; row++ ) {
      for( col = 1; col <= p_board->cols; col++ ) {
        DrawMine( hdc, hMemDC, p_board, col, row, FALSE );
      }
    }
    SelectObject( hMemDC, hOldObj );
}

static void DrawLeds( HDC hdc, HDC hMemDC, BOARD *p_board, int number, int x, int y )
{
    HGDIOBJ hOldObj;
    unsigned led[3], i;
    int count;

    count = number;
    if( count < 1000 ) {
        if( count >= 0 ) {
            led[0] = count / 100 ;
            count -= led[0] * 100;
        }
        else {
            led[0] = 10; /* negative sign */
            count = -count;
        }
        led[1] = count / 10;
        count -= led[1] * 10;
        led[2] = count;
    }
    else {
        for( i = 0; i < 3; i++ )
            led[i] = 10;
    }

    hOldObj = SelectObject (hMemDC, p_board->hLedsBMP);

    for( i = 0; i < 3; i++ ) {
        BitBlt( hdc,
            i * LED_WIDTH + x,
            y,
            LED_WIDTH,
            LED_HEIGHT,
            hMemDC,
            0,
            led[i] * LED_HEIGHT,
            SRCCOPY);
    }

    SelectObject( hMemDC, hOldObj );
}


static void DrawFace( HDC hdc, HDC hMemDC, BOARD *p_board )
{
    HGDIOBJ hOldObj;

    hOldObj = SelectObject (hMemDC, p_board->hFacesBMP);

    BitBlt( hdc,
        p_board->face_rect.left,
        p_board->face_rect.top,
        FACE_WIDTH,
        FACE_HEIGHT,
        hMemDC, 0, p_board->face_bmp * FACE_HEIGHT, SRCCOPY);

    SelectObject( hMemDC, hOldObj );
}


static void DrawBoard( HDC hdc, HDC hMemDC, PAINTSTRUCT *ps, BOARD *p_board )
{
    RECT tmp_rect;

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->counter_rect ) )
        DrawLeds( hdc, hMemDC, p_board, p_board->mines - p_board->num_flags,
                  p_board->counter_rect.left,
                  p_board->counter_rect.top );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->timer_rect ) )
        DrawLeds( hdc, hMemDC, p_board, p_board->time,
                  p_board->timer_rect.left,
                  p_board->timer_rect.top );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->face_rect ) )
        DrawFace( hdc, hMemDC, p_board );

    if( IntersectRect( &tmp_rect, &ps->rcPaint, &p_board->mines_rect ) )
        DrawMines( hdc, hMemDC, p_board );
}

static void UnpressBox( BOARD *p_board, unsigned col, unsigned row )
{
    HDC hdc;
    HGDIOBJ hOldObj;
    HDC hMemDC;

    hdc = GetDC( p_board->hWnd );
    hMemDC = CreateCompatibleDC( hdc );
    hOldObj = SelectObject( hMemDC, p_board->hMinesBMP );

    DrawMine( hdc, hMemDC, p_board, col, row, FALSE );

    SelectObject( hMemDC, hOldObj );
    DeleteDC( hMemDC );
    ReleaseDC( p_board->hWnd, hdc );
}


static void UnpressBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        UnpressBox( p_board, col + i, row + j );
      }
}


static void PressBox( BOARD *p_board, unsigned col, unsigned row )
{
    HDC hdc;
    HGDIOBJ hOldObj;
    HDC hMemDC;

    hdc = GetDC( p_board->hWnd );
    hMemDC = CreateCompatibleDC( hdc );
    hOldObj = SelectObject (hMemDC, p_board->hMinesBMP);

    DrawMine( hdc, hMemDC, p_board, col, row, TRUE );

    SelectObject( hMemDC, hOldObj );
    DeleteDC( hMemDC );
    ReleaseDC( p_board->hWnd, hdc );
}


static void PressBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        p_board->box[col + i][row + j].IsPressed = TRUE;
        PressBox( p_board, col + i, row + j );
    }

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        if( !p_board->box[p_board->press.x + i][p_board->press.y + j].IsPressed )
            UnpressBox( p_board, p_board->press.x + i, p_board->press.y + j );
    }

    for( i = -1; i <= 1; i++ )
      for( j = -1; j <= 1; j++ ) {
        p_board->box[col + i][row + j].IsPressed = FALSE;
        PressBox( p_board, col + i, row + j );
    }

    p_board->press.x = col;
    p_board->press.y = row;
}


static void CompleteBox( BOARD *p_board, unsigned col, unsigned row )
{
    int i, j;

    if(col > 0 && col < p_board->cols + 1 &&
            row > 0 && row < p_board->rows + 1 ) {
        p_board->box[col][row].FlagType = COMPLETE;

        p_board->face_bmp = DEAD_BMP;
        p_board->boxes_left--;
    }
}


static void CompleteBoxes( BOARD *p_board, unsigned col, unsigned row )
{
    CompleteBox( p_board, col, row );
}

static void TestMines( BOARD *p_board, POINT pt, int msg )
{
    BOOL draw = TRUE;
    int col, row;

    col = (pt.x - p_board->mines_rect.left) / MINE_WIDTH + 1;
    row = (pt.y - p_board->mines_rect.top ) / MINE_HEIGHT + 1;

    switch ( msg ) {
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        if( p_board->press.x != col || p_board->press.y != row ) {
            UnpressBox( p_board,
                    p_board->press.x, p_board->press.y );
            p_board->press.x = col;
            p_board->press.y = row;
            PressBox( p_board, col, row );
        }
        draw = FALSE;
        break;

    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        if( p_board->press.x != col || p_board->press.y != row )
            UnpressBox( p_board,
                    p_board->press.x, p_board->press.y );
        p_board->press.x = 0;
        p_board->press.y = 0;
        p_board->status = PLAYING;
        PlaceMines( p_board, col, row );
        CompleteBox( p_board, col, row );
        break;

    default:
        break;
    }

    if( draw )
    {
        RedrawWindow( p_board->hWnd, NULL, 0,
            RDW_INVALIDATE | RDW_UPDATENOW );
    }
}


static void TestFace( BOARD *p_board, POINT pt, int msg )
{
    if( p_board->status == PLAYING || p_board->status == WAITING ) {
        if( msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN )
            p_board->face_bmp = OOH_BMP;
        else p_board->face_bmp = SMILE_BMP;
    }
    else if( p_board->status == GAMEOVER )
        p_board->face_bmp = DEAD_BMP;
    else if( p_board->status == WON )
            p_board->face_bmp = COOL_BMP;

    if( PtInRect( &p_board->face_rect, pt ) ) {
        if( msg == WM_LBUTTONDOWN )
            p_board->face_bmp = SPRESS_BMP;

        if( msg == WM_LBUTTONUP )
            CreateBoard( p_board );
    }

    RedrawWindow( p_board->hWnd, &p_board->face_rect, 0,
        RDW_INVALIDATE | RDW_UPDATENOW );
}


static void TestBoard( HWND hWnd, BOARD *p_board, int x, int y, int msg )
{
    POINT pt;
    unsigned col,row;

    pt.x = x;
    pt.y = y;

    if( PtInRect( &p_board->mines_rect, pt ) )
        TestMines( p_board, pt, msg );
    else {
        UnpressBoxes( p_board,
            p_board->press.x,
            p_board->press.y );
        p_board->press.x = 0;
        p_board->press.y = 0;
    }

    TestFace( p_board, pt, msg );
}


static LRESULT WINAPI MainProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HMENU hMenu;
    static BOARD board;

    switch( msg ) {
    case WM_CREATE:
        board.hInst = ((LPCREATESTRUCTW) lParam)->hInstance;
        board.hWnd = hWnd;
        InitBoard( &board );
        CreateBoard( &board );
        return 0;

    case WM_PAINT:
      {
        HDC hMemDC;

        hdc = BeginPaint( hWnd, &ps );
        hMemDC = CreateCompatibleDC( hdc );

        DrawBoard( hdc, hMemDC, &ps, &board );

        DeleteDC( hMemDC );
        EndPaint( hWnd, &ps );

        return 0;
      }

    case WM_MOVE:
        board.pos.x = (short)LOWORD(lParam);
        board.pos.y = (short)HIWORD(lParam);
        return 0;

    case WM_DESTROY:
        SaveBoard( &board );
        DestroyBoard( &board );
        PostQuitMessage( 0 );
        return 0;

    case WM_TIMER:
       board.time++;
	    RedrawWindow( hWnd, &board.timer_rect, 0,
			  RDW_INVALIDATE | RDW_UPDATENOW );
        return 0;

    case WM_LBUTTONDOWN:
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        SetCapture( hWnd );
        return 0;

    case WM_LBUTTONUP:
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        ReleaseCapture();
        return 0;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam), msg );
        return 0;

    case WM_MOUSEMOVE:
    {
        if( ( wParam & MK_MBUTTON ) ||
            ( ( wParam & MK_LBUTTON ) && ( wParam & MK_RBUTTON ) ) ) {
            msg = WM_MBUTTONDOWN;
        }
        else if( wParam & MK_LBUTTON ) {
            msg = WM_LBUTTONDOWN;
        }
        else {
            return 0;
        }

        TestBoard( hWnd, &board, (short)LOWORD(lParam), (short)HIWORD(lParam),  msg );

        return 0;
    }

    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case IDM_NEW:
            CreateBoard( &board );
            return 0;

        case IDM_MARKQ:
            hMenu = GetMenu( hWnd );
            board.IsMarkQ = !board.IsMarkQ;
            if( board.IsMarkQ )
                CheckMenuItem( hMenu, IDM_MARKQ, MF_CHECKED );
            else
                CheckMenuItem( hMenu, IDM_MARKQ, MF_UNCHECKED );
            return 0;

        case IDM_BEGINNER:
            SetDifficulty( &board, BEGINNER );
            CreateBoard( &board );
            return 0;

        case IDM_ADVANCED:
            SetDifficulty( &board, ADVANCED );
            CreateBoard( &board );
            return 0;

        case IDM_EXPERT:
            SetDifficulty( &board, EXPERT );
            CreateBoard( &board );
            return 0;

        case IDM_CUSTOM:
            SetDifficulty( &board, CUSTOM );
            CreateBoard( &board );
            return 0;

        case IDM_EXIT:
            SendMessageW( hWnd, WM_CLOSE, 0, 0);
            return 0;

        case IDM_TIMES:
            DialogBoxParamW( board.hInst, MAKEINTRESOURCEW(DLG_TIMES), hWnd,
                             TimesDlgProc, (LPARAM) &board);
            return 0;

        case IDM_ABOUT:
        {
            WCHAR appname[256], other[256];
            LoadStringW( board.hInst, IDS_APPNAME, appname, 256);
            LoadStringW( board.hInst, IDS_ABOUT, other, 256);
            ShellAboutW( hWnd, appname, other,
                         LoadImageW(board.hInst, MAKEINTRESOURCEW(IDI_WINEMINE), IMAGE_ICON, 48, 48, LR_SHARED));
            return 0;
        }
        default:
            break;
        }
    }
    return DefWindowProcW( hWnd, msg, wParam, lParam );
}

void start()
{
    MSG msg;
    WNDCLASSEXW wc;
    HWND hWnd;
    HACCEL haccel;
    WCHAR appname[20];
	HINSTANCE hInst = GetModuleHandle(NULL);
	if (!hInst) ExitProcess(1);
	int cmdshow = SW_SHOW;

    LoadStringW( hInst, IDS_APPNAME, appname, 20);

    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = MainProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIconW( hInst, MAKEINTRESOURCEW(IDI_WINEMINE) );
    wc.hCursor = LoadCursorW( 0, (LPWSTR)IDI_APPLICATION );
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE); // Mod to match ReactOS
    wc.lpszMenuName = MAKEINTRESOURCEW(IDM_WINEMINE);
    wc.lpszClassName = appname;
    wc.hIconSm = LoadImageW( hInst, MAKEINTRESOURCEW(IDI_WINEMINE), IMAGE_ICON,
                            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED );

    if (!RegisterClassExW(&wc)) ExitProcess(1);
    hWnd = CreateWindowW( appname, appname,
	wnd_style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, hInst, NULL );

    if (!hWnd) ExitProcess(1);

    ShowWindow( hWnd, cmdshow );
    UpdateWindow( hWnd );

    haccel = LoadAcceleratorsW( hInst, MAKEINTRESOURCEW(IDA_WINEMINE) );
    SetTimer( hWnd, ID_TIMER, 1000, NULL );

    while( GetMessageW(&msg, 0, 0, 0) ) {
        if (!TranslateAcceleratorW( hWnd, haccel, &msg ))
            TranslateMessage( &msg );

        DispatchMessageW( &msg );
    }
    ExitProcess(msg.wParam);
}
