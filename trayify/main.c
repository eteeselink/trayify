#include "stdafx.h"
#include <stdio.h>
#include "stretchy_buffer.h"
#include <conio.h>

typedef struct {
    DWORD processId;
    HWND* handles;
} FilteredWindows;

static BOOL CALLBACK FilterWindow( HWND hwnd, LPARAM lParam )
{
    FilteredWindows *args = (FilteredWindows *)lParam;
    DWORD windowPID;

    GetWindowThreadProcessId(hwnd, &windowPID);
    if (windowPID == args->processId) 
    {
        sbpush(args->handles, hwnd);
    }

    return TRUE;
}

HWND* GetWindowsForProcess(int processId)
{
    FilteredWindows result;
    
    result.processId = processId;
    result.handles = NULL;

    if (EnumWindows(&FilterWindow, (LPARAM) &result ) == FALSE ) {
      return NULL;
    }
    return result.handles;
}

/** Waits for the process to spawn at least one window, and returns the handle to one such window. */
HWND WaitForWindow(HANDLE process)
{
    DWORD processId;
    HWND* handles;
    int len = 0;

    processId = GetProcessId(process);
    printf("\n pid: %d", processId);

    while(len == 0)
    {
        handles = GetWindowsForProcess(processId);
        len = sbcount(handles);
        Sleep(200);
    }

    return handles[0];
}

#define CMDLINE_BUF_SIZE 200
HANDLE ExecuteCmdLine(int argc, char** argv)
{
    BOOL ok;
    char cmdLine[CMDLINE_BUF_SIZE];
    int i;
    SHELLEXECUTEINFO shellExecuteInfo;

    memset(&shellExecuteInfo, 0, sizeof(shellExecuteInfo));
    shellExecuteInfo.cbSize = sizeof(shellExecuteInfo);

    cmdLine[0] = '\0';
    for(i = 2; i < argc; i++)
    {
        strcat_s(cmdLine, CMDLINE_BUF_SIZE, argv[i]);
        strcat_s(cmdLine, CMDLINE_BUF_SIZE, " ");
    }

    shellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellExecuteInfo.lpVerb = "open";
    shellExecuteInfo.lpFile = argv[1];
    shellExecuteInfo.lpParameters = cmdLine;
    shellExecuteInfo.nShow = SW_SHOWDEFAULT;
    ok = ShellExecuteEx(&shellExecuteInfo);
    if(!ok)
    {
        printf("Error: %d\n", GetLastError());
        exit(-1);
    }

    return shellExecuteInfo.hProcess;
}

typedef struct 
{
    HWND hwnd;
    HWND targetWnd;
} MainWindow;


#define LONGPTR_WINDOW 0
#define TIMER_POLL 0

void MainWindow_CheckTarget(MainWindow* win)
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(win->targetWnd, &wp);

    printf("Checking...\n");
    if(IsWindowVisible(win->targetWnd))
    {
        // if we didn't hide the window before, should we hide it now?
        switch(wp.showCmd)
        {
        case SW_MINIMIZE:
        case SW_SHOWMINNOACTIVE:
        case SW_SHOWMINIMIZED:
            // user minimized the window, so let's hide it.
            printf("Hiding!\n" );
            ShowWindow(win->targetWnd, SW_HIDE);
            break;
        }
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
	case WM_TIMER:
        printf("Got timer ping!\n");
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
        printf("CreateWindowEx %d\n", GetLastError());
        return FALSE;
    }
    
    SetWindowLongPtr(hwnd, LONGPTR_WINDOW, (LONG)win);
    //if(!)
    //{
    //   printf("SetWindowLongPtr %d\n", GetLastError());
    //    return FALSE;
    //}
    
    SetTimer(hwnd, TIMER_POLL, 200, NULL);
    printf("Created main window, starting timer\n");

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

int main(int argc, char** argv)
{
    HWND targetWnd;
    MainWindow mainWindow;
    HANDLE process;

    process = ExecuteCmdLine(argc, argv);
    targetWnd = WaitForWindow(process);
    
    {
        char buf[100];
        GetWindowText(targetWnd, buf, 100);
        //ShowWindow(handles[i], SW_HIDE);
        printf("\n  Found: %s\n", buf);
    }

	MainWindow_Create(&mainWindow, targetWnd);
    
	MainWindow_Mainloop(&mainWindow);
    
    while(!_kbhit()) { Sleep(50); }

    return 0;
}