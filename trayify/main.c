#include "stdafx.h"
#include <stdio.h>
#include "stretchy_buffer.h"

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
        printf("Error: %d", GetLastError());
        exit(-1);
    }

    return shellExecuteInfo.hProcess;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
            //case WM_COMMAND:
              // handle menu selections etc.
            //break;
            //case WM_PAINT:
              // draw our window - note: you must paint something here or not trap it!
            //break;
            case WM_DESTROY:
                 PostQuitMessage(0);
            break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HWND CreateMainWindow()
{
    static const char* class_name = "TRAYIFY_MESSAGE_WND";
    WNDCLASSEX wx;
    HWND mainWnd = 0;

    memset(&wx, 0, sizeof(wx));
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = WndProc;        // function which will handle messages
    wx.hInstance = (HINSTANCE)GetModuleHandle(NULL);
    wx.lpszClassName = class_name;

    if(RegisterClassEx(&wx)) 
    {
        mainWnd = CreateWindowEx( 0, class_name, "Trayify Message Window", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL );
    }

    return mainWnd;
}

int main(int argc, char** argv)
{
    HANDLE process;
    HWND targetWnd;

    process = ExecuteCmdLine(argc, argv);
    targetWnd = WaitForWindow(process);
    
    {
        char buf[100];
        GetWindowText(targetWnd, buf, 100);
        //ShowWindow(handles[i], SW_HIDE);
        printf("\n  Found: %s", buf);
    }


    return 0;
}