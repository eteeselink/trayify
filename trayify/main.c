#include "stdafx.h"
#include <stdio.h>
#include "stretchy_buffer.h"
#include "stolen_code.h"
#include "MainWindow.h"

#ifdef DEBUG
void log(char* fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
}
#else
void log(char* fmt, ...)
{ }
#endif

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
    log("\n pid: %d", processId);

    while(len == 0)
    {
        handles = GetWindowsForProcess(processId);
        len = sbcount(handles);
        Sleep(200);
    }

    return handles[0];
}

#define CMDLINE_BUF_SIZE 200
HANDLE ExecuteCmdLine(int argc, const char** argv)
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
        log("Error: %d\n", GetLastError());
        exit(-1);
    }

    return shellExecuteInfo.hProcess;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND targetWnd;
    MainWindow mainWindow;
    HANDLE process;

    LPCSTR *argv;
    int argc;

    argv = CommandLineToArgvA(GetCommandLineA(), &argc);
    if( NULL == argv )
    {
        printf("CommandLineToArgvW failed\n");
        return 0;
    }

    process = ExecuteCmdLine(argc, argv);
    targetWnd = WaitForWindow(process);
    
    {
        char buf[100];
        GetWindowText(targetWnd, buf, 100);
        log("\n  Found: %s\n", buf);
    }

	MainWindow_Create(&mainWindow, targetWnd);
    MainWindow_CreateTrayIcon(&mainWindow);
	MainWindow_Mainloop(&mainWindow);
    
    return 0;
}