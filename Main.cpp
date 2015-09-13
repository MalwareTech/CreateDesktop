#include <Windows.h>
#include <stdio.h>

//Create a new desktop or open an existing one
HDESK CreateHiddenDesktop(CHAR *desktop_name)
{
    CHAR explorer_path[MAX_PATH];
    HDESK hidden_desktop = NULL, original_desktop;
    STARTUPINFOA startup_info = {0};
    PROCESS_INFORMATION process_info = {0};

    ExpandEnvironmentStringsA("%windir%\\explorer.exe", explorer_path, MAX_PATH-1);

    hidden_desktop = OpenDesktopA(desktop_name, NULL, FALSE, GENERIC_ALL);
    if(!hidden_desktop)
    {
        hidden_desktop = CreateDesktopA(desktop_name, NULL, NULL, 0, GENERIC_ALL, NULL);
        if(hidden_desktop)
        {
            original_desktop = GetThreadDesktop(GetCurrentThreadId());

            if(SetThreadDesktop(hidden_desktop))
            {
                startup_info.cb = sizeof(startup_info);
                startup_info.lpDesktop = desktop_name;

                //We need to create an explorer.exe in the context of the new desktop for start menu, etc
                CreateProcessA(explorer_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info);

                SetThreadDesktop(original_desktop);
            }
        }
    }
    return hidden_desktop;
}

void main()
{
    HDESK original_desktop, hidden_desktop;
    
    hidden_desktop = CreateHiddenDesktop("MalwareTech_Hidden");
    original_desktop = GetThreadDesktop(GetCurrentThreadId());

    printf("Entering hidden desktop\n");

    //Need to switch thread into context of new desktop to register hotkeys
    SetThreadDesktop(hidden_desktop);
    SwitchDesktop(hidden_desktop);

    if(RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 0x45))
    {
        MSG msg = {0};
        while(GetMessage(&msg, NULL, 0, 0) != 0)
        {
            if(msg.message == WM_HOTKEY)
            {
                printf("Exiting hidden desktop\n");
                SwitchDesktop(original_desktop);
                break;
            }
        }
    }

    CloseHandle(hidden_desktop);
    getchar();
}