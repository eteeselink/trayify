#include "stdafx.h"

typedef struct 
{
    HWND hwnd;
    HWND targetWnd;
    NOTIFYICONDATA nid;
} MainWindow;


#define LONGPTR_WINDOW 0
#define TIMER_POLL 0
#define WM_TRAYIFY_NOTIFY (WM_APP + 1)

void             MainWindow_CreateTrayIcon(MainWindow* win);
void             MainWindow_CheckTarget(MainWindow* win);
void             MainWindow_HandleNotifyEvent(MainWindow* win, WPARAM id, LPARAM message);
LRESULT CALLBACK MainWindow_WndProc(MainWindow* win, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL             MainWindow_Create(MainWindow* win, HWND targetWnd);
void             MainWindow_Mainloop(MainWindow* win);