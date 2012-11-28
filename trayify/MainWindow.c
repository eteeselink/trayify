#include "stdafx.h"

#include "MainWindow.h"

void MainWindow_CreateTrayIcon(MainWindow* win)
{
    memset(&(win->nid), 0, sizeof(win->nid));

    win->nid.cbSize = sizeof(NOTIFYICONDATA);
    win->nid.hWnd = win->hwnd;
    win->nid.uID = 100;
    win->nid.uVersion = NOTIFYICON_VERSION;
    win->nid.uCallbackMessage = WM_TRAYIFY_NOTIFY;
    win->nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    //strcpy_s(win->nid.szTip, 4, "Moo!");
    win->nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
     
    Shell_NotifyIcon(NIM_ADD, &win->nid);
}

void MainWindow_CheckTarget(MainWindow* win)
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(win->targetWnd, &wp);

    log("Checking...\n");
    if(IsWindowVisible(win->targetWnd))
    {
        // if we didn't hide the window before, should we hide it now?
        switch(wp.showCmd)
        {
        case SW_MINIMIZE:
        case SW_SHOWMINNOACTIVE:
        case SW_SHOWMINIMIZED:
            // user minimized the window, so let's hide it.
            log("Hiding!\n" );
            //MainWindow_TargetToTray(win);
            ShowWindow(win->targetWnd, SW_HIDE);
            break;
        }
    }
}
        
void MainWindow_HandleNotifyEvent(MainWindow* win, WPARAM id, LPARAM message)
{
    switch(message)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if(!IsWindowVisible(win->targetWnd))
        {
            ShowWindow(win->targetWnd, SW_RESTORE);
            SetForegroundWindow(win->targetWnd);
        }
        break;
    }
}

LRESULT CALLBACK MainWindow_WndProc(MainWindow* win, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        //case WM_COMMAND:
            // handle menu selections etc.
        //break;
        //case WM_PAINT:
            // draw our window - note: you must paint something here or not trap it!
        //break;
    case WM_TRAYIFY_NOTIFY:
        log("oooh!\n");
        MainWindow_HandleNotifyEvent(win, wParam, lParam);
        break;
	case WM_TIMER:
        log("Got timer ping!\n");
        switch(wParam)
        {
        case TIMER_POLL:
            MainWindow_CheckTarget(win);
            break;
        }
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow* win = (MainWindow*)GetWindowLongPtr(hwnd, LONGPTR_WINDOW);
    return MainWindow_WndProc(win, hwnd, message, wParam, lParam);
    //return DefWindowProc(hwnd, message, wParam, lParam);
}



BOOL MainWindow_Create(MainWindow* win, HWND targetWnd)
{
    static const char* class_name = "TRAYIFY_MESSAGE_WND";
    WNDCLASSEX wx;
    HWND hwnd;

    memset(&wx, 0, sizeof(wx));
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = WndProc;       
    wx.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wx.lpszClassName = class_name;
    wx.cbWndExtra = sizeof(MainWindow*) + sizeof(int);

    if(!RegisterClassEx(&wx)) 
    {
        return FALSE;
    }
    
    hwnd = CreateWindowEx( 0, class_name, "Trayify Message Window", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );
    if(hwnd == 0)
    {
        log("CreateWindowEx %d\n", GetLastError());
        return FALSE;
    }
    
    SetWindowLongPtr(hwnd, LONGPTR_WINDOW, (LONG)win);
    //if(!)
    //{
    //   printf("SetWindowLongPtr %d\n", GetLastError());
    //    return FALSE;
    //}
    
    SetTimer(hwnd, TIMER_POLL, 200, NULL);
    log("Created main window, starting timer\n");

    win->hwnd = hwnd;
    win->targetWnd = targetWnd;
    return TRUE;
}

void MainWindow_Mainloop(MainWindow* win)
{
    MSG msg;
    while(GetMessage(&msg, win->hwnd, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
