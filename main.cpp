#include <windows.h>
#include <iostream>
#include <stdint.h>
#include <tlhelp32.h>
#include <vector>

void ExitWithErr(const wchar_t*);

DWORD WINAPI ThreadProc(LPVOID lpParameter);
void InfiniteAmmo(HANDLE  hProcess , uintptr_t ammoAdress);

DWORD GetProcId(LPCTSTR ProcessName);
uintptr_t GetModuleBaseAddress(DWORD procId , LPCTSTR ProcessName);
uintptr_t FindAdressWithOffsets(HANDLE hproc , uintptr_t modBaseAdress , std::vector <unsigned int> offsets);

typedef struct tParams {
    HANDLE hProcess;
    uintptr_t adress;
} tParams, *PtParams;

// Define the ptr to the struct wich will be contains args for the threads
PtParams PhealthParams;
PtParams PammoParams;


//Define bool var to check if some function are activated
bool run = true;
bool infiniteHealthEnable = false;
bool infiniteAmmoEnable = false;

//Offsets array
std::vector <unsigned int> ammoOffsets = { 0x374 , 0x14 , 0x0 };
std::vector <unsigned int> healthOffsets = { 0xf8 };

//Define the threads/process id
DWORD procId;
DWORD tIdInfiniteHealth;
DWORD tIdInfiniteAmmo;

//Define the handle of the threads/process
HANDLE hProcess;
HANDLE tInfiniteHealth;
HANDLE tInfiniteAmmo;

//Define the threads exit codes
DWORD tInfiniteHealthExitCode;
DWORD tInfiniteAmmoExitCode;

//Define pointer to contains/resolve ammo and health adress
uintptr_t moduleBase;
uintptr_t dynamicPtrBaseAdress;
uintptr_t ammoDynamicAdress;
uintptr_t healthDynamicAdress;

//Define security attributes of the threads
_SECURITY_ATTRIBUTES tSecurityAttr;

int main()
{
    tSecurityAttr.nLength = sizeof(tSecurityAttr);
    tSecurityAttr.lpSecurityDescriptor = NULL;
    tSecurityAttr.bInheritHandle = false;

    //Get ac_client.exe id
    procId = GetProcId(TEXT("ac_client.exe"));

    //Get ac_client.exe base adress
    moduleBase = GetModuleBaseAddress(procId , TEXT("ac_client.exe"));

    //Print base adress 
    std::cout << "ac_client.exe base adress :\t0x" << std::hex << moduleBase << std::endl;

    //Calculate the localPlayerAdress
    dynamicPtrBaseAdress = moduleBase + 0x10f4f4;

    //Open the process
    hProcess = OpenProcess(PROCESS_ALL_ACCESS , false , procId);

    //Print menu
    std::cout << "infinite health [1] " << std::endl;
    std::cout << "infinite ammo   [2] " << std::endl;
    std::cout << "exit            [3] " << std::endl;

    do {
        if(GetAsyncKeyState(VK_NUMPAD1))
        {
            if (infiniteHealthEnable == false)
            {
                    //Allocate memory on the heap for the params
                    PhealthParams = (PtParams) HeapAlloc(GetProcessHeap() , HEAP_ZERO_MEMORY , sizeof(tParams));
                    
                    if(PhealthParams == NULL)
                    {
                        std::cout << "InfiniteHealth(HeapAlloc)Can't allocate memory : " << GetLastError() << std::endl;

                    } else {

                        //Passing args to the struct
                        PhealthParams->hProcess = hProcess;
                        PhealthParams->adress = FindAdressWithOffsets(hProcess , dynamicPtrBaseAdress , healthOffsets);

                        //Create thread
                        tInfiniteHealth = CreateThread( &tSecurityAttr,0, (LPTHREAD_START_ROUTINE)ThreadProc,PhealthParams,0,&tIdInfiniteHealth);

                        if (tInfiniteHealth == NULL)
                        {
                            std::cout << "Can't create the thread to get infinite health : " << GetLastError() << std::endl;
                        }

                        //Now the thread is created :)
                        infiniteHealthEnable = true;
                        std::cout << "Infinite health enable" << std::endl;
                    }

            } else {
                //Need to close the thread

                //Get the exit code
                if (GetExitCodeThread(tInfiniteHealth , &tInfiniteHealthExitCode))
                {
                    //Close thread
                    if(TerminateThread(tInfiniteHealth , tInfiniteHealthExitCode))
                    {
                        CloseHandle(tInfiniteHealth);
                        //Free the heap
                        if(!HeapFree(GetProcessHeap() , 0 , PhealthParams))
                        {
                            ExitWithErr(L"InfiniteHealth->CloseThread(HeapFree)");
                        }

                        infiniteHealthEnable = false;
                        std::cout << "Infinite health disabled" << std::endl;

                    } else {
                        ExitWithErr(L"InfiniteHealth->CloseThread(TerminateThread)");
                    }
                } else {
                    ExitWithErr(L"InfiniteHealth->CloseThread(GetExitCode)");
                }
            }
        }
        if(GetAsyncKeyState(VK_NUMPAD2))
        {
            if (infiniteAmmoEnable == false)
            {
                    //Allocate memory on the heap for the params
                    PammoParams = (PtParams) HeapAlloc(GetProcessHeap() , HEAP_ZERO_MEMORY , sizeof(tParams));
                    
                    if(PammoParams == NULL)
                    {
                        std::cout << "InfiniteAmmo(HeapAlloc)Can't allocate memory : " << GetLastError() << std::endl;

                    } else {

                        //Passing args to the struct
                        PammoParams->hProcess = hProcess;
                        PammoParams->adress = FindAdressWithOffsets(hProcess , dynamicPtrBaseAdress , ammoOffsets);

                        //Create thread
                        tInfiniteAmmo = CreateThread( &tSecurityAttr,0, (LPTHREAD_START_ROUTINE)ThreadProc,PammoParams,0,&tIdInfiniteAmmo);

                        if (tInfiniteAmmo == NULL)
                        {
                            std::cout << "Can't create the thread to get infinite ammo : " << GetLastError() << std::endl;
                        }

                        //Now the thread is created :)
                        std::cout << "Infinite ammo enable" << std::endl;
                        infiniteAmmoEnable = true;
                    }

            } else if (infiniteAmmoEnable == true) {
                //Need to close the thread
                if (GetExitCodeThread(tInfiniteAmmo , &tInfiniteAmmoExitCode))
                {
                    if(TerminateThread(tInfiniteAmmo , tInfiniteAmmoExitCode))
                    {
                        CloseHandle(tInfiniteAmmo);
                        if(!HeapFree(GetProcessHeap() , 0 , PammoParams))
                        {
                            ExitWithErr(L"InfiniteAmmo->CloseThread(HeapFree)");
                        }

                        infiniteAmmoEnable = false;
                        std::cout << "Infinite ammo disabled" << std::endl;

                    } else {
                        ExitWithErr(L"InfiniteAmmo->CloseThread(TerminateThread)");
                    }
                } else {
                    ExitWithErr(L"InfiniteAmmo->CloseThread(GetExitCode)");
                }
            }
        }
        if(GetAsyncKeyState(VK_NUMPAD3))
        {  
            CloseHandle(tInfiniteHealth);
            CloseHandle(tInfiniteAmmo);
            CloseHandle(hProcess);
            run = false;
            std::cout << "Bye bye ..." << std::endl; 
        }
        Sleep(250);
    } while(run);
    

    CloseHandle(hProcess);
    return 0;
}
uintptr_t GetModuleBaseAddress(DWORD procId , LPCTSTR moduleName) 
{   

    uintptr_t  modBaseAdress;

    //Create a snapshot of every module of a process 
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE , procId);


    //Declare the struct wich will be contain the information about a module
    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);

    //Check if the first Module have been copied to the buffer
    if (!Module32First( hSnap , &modEntry)) 
    {
        ExitWithErr(L"GetModuleBaseAddress(Module32First)");
    }

    //Iterate all module 
    do {
        //Find the module searched and get his base adress
        if (lstrcmpi(modEntry.szModule, moduleName) == 0) 
        {   
            modBaseAdress = (uintptr_t)modEntry.modBaseAddr;  
            break;
        } 


    } while (Module32Next(hSnap , &modEntry));

    CloseHandle(hSnap);
    return modBaseAdress;
}
uintptr_t FindAdressWithOffsets(HANDLE hproc , uintptr_t modBaseAdress , std::vector <unsigned int> offsets) 
{
    uintptr_t addr = modBaseAdress;

    for (unsigned int i = 0 ; i < offsets.size() ; i++ ) 
    {   
        //RRM (in , in , out , in , out)
        ReadProcessMemory(hproc , (BYTE*)addr , &addr , sizeof(addr) , 0);
        addr += offsets[i];
    }

    return addr;
}
DWORD GetProcId(LPCTSTR ProcessName)
{
    PROCESSENTRY32 pt;
    HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pt.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hsnap, &pt)) { 
        do {
            if (!lstrcmpi(pt.szExeFile, ProcessName)) {
                CloseHandle(hsnap);
                return pt.th32ProcessID;
            }
        } while (Process32Next(hsnap, &pt));
    }
    CloseHandle(hsnap); 
    return -1;
}
DWORD WINAPI ThreadProc(LPVOID lpParameter) 
{
    int newValue = 999;

    PtParams pInfo = (PtParams)lpParameter;
    uintptr_t adressToWrite = pInfo->adress;
    HANDLE hProcess = pInfo->hProcess;
    
    while(true)
    {
        if(WriteProcessMemory(hProcess , (LPVOID)adressToWrite , &newValue , sizeof(newValue) , NULL) == 0)
        {
            std::cout << "\nCan't write att : 0x" << std::hex << adressToWrite <<std::endl;
        } 
    }
}
void ExitWithErr(const wchar_t* caller) 
{
    std::cout << "Error from " << caller << " : " << GetLastError() << std::endl;
    exit(-1); 
}