#include "../../src/tesmio_plugin.h"
#include <windows.h>
#include <stdio.h>

#define RVA_TRAFFIC_LIGHT_TICK 0x225BD0

typedef void (*FnTrafficLightTick)(void* junc, float deltaTime);
static FnTrafficLightTick g_origTrafficLightTick = NULL;

struct TrafficLightPhase {
    int roadIdx1;
    int roadIdx2;
    float duration;
};

static void Hook_TrafficLightTick(void* junc, float deltaTime)
{
    if (!junc) return;
    
    // Check if it's actually in Traffic Light mode (mode == 2 at offset 0)
    int mode = *(int*)junc;
    if (mode == 2)
    {
        // Read current phase index
        int phaseIdx = *(int*)((BYTE*)junc + 0xD8);
        if (phaseIdx >= 0)
        {
            BYTE* phaseVecStart = *(BYTE**)((BYTE*)junc + 0xB8);
            BYTE* phaseVecEnd = *(BYTE**)((BYTE*)junc + 0xC0);
            int phaseCount = (phaseVecEnd - phaseVecStart) / 12;
            
            if (phaseIdx < phaseCount)
            {
                TrafficLightPhase* curPhase = (TrafficLightPhase*)(phaseVecStart + phaseIdx * 12);
                
                // Read incoming roads array
                BYTE* roadsStart = *(BYTE**)((BYTE*)junc + 0xA0);
                BYTE* roadsEnd = *(BYTE**)((BYTE*)junc + 0xA8);
                int roadCount = (roadsEnd - roadsStart) / 0x1D8;
                
                bool hasCarsWaiting = false;
                bool anyCarsOnJunction = false;
                
                // First, check if there are ANY cars waiting at the entire junction
                for (int i = 0; i < roadCount; i++) {
                    BYTE* road = roadsStart + (i * 0x1D8);
                    BYTE* vehVecStart = *(BYTE**)(road + 0x1A8);
                    BYTE* vehVecEnd = *(BYTE**)(road + 0x1B0);
                    if (vehVecEnd > vehVecStart) {
                        anyCarsOnJunction = true;
                        break;
                    }
                }
                
                // Check if cars are waiting on the roads that currently have green
                bool isGreenPhase = false;
                
                if (curPhase->roadIdx1 >= 0 && curPhase->roadIdx1 < roadCount) {
                    isGreenPhase = true;
                    BYTE* road = roadsStart + (curPhase->roadIdx1 * 0x1D8);
                    BYTE* vehVecStart = *(BYTE**)(road + 0x1A8);
                    BYTE* vehVecEnd = *(BYTE**)(road + 0x1B0);
                    if (vehVecEnd > vehVecStart) {
                        hasCarsWaiting = true;
                    }
                }
                
                if (curPhase->roadIdx2 >= 0 && curPhase->roadIdx2 < roadCount) {
                    isGreenPhase = true;
                    BYTE* road = roadsStart + (curPhase->roadIdx2 * 0x1D8);
                    BYTE* vehVecStart = *(BYTE**)(road + 0x1A8);
                    BYTE* vehVecEnd = *(BYTE**)(road + 0x1B0);
                    if (vehVecEnd > vehVecStart) {
                        hasCarsWaiting = true;
                    }
                }
                
                // If NO cars are waiting for this green phase, BUT there are cars waiting somewhere else, skip!
                // IMPORTANT: We only skip if isGreenPhase is true. If both indices are -1, it's a yellow/red transition phase!
                if (isGreenPhase && !hasCarsWaiting && anyCarsOnJunction) {
                    // Set current timer to the duration so it wraps immediately
                    *(float*)((BYTE*)junc + 0xDC) = curPhase->duration + 1.0f;
                    // Provide a small delta time so it triggers the swap condition inside original function
                    deltaTime = 0.1f;
                }
            }
        }
    }

    g_origTrafficLightTick(junc, deltaTime);
}

static void ApplyPatches(void)
{
    BYTE* base = (BYTE*)g_exeBase;
    
    static const BYTE kExpect[19] = {
        0x48, 0x83, 0xEC, 0x28, 
        0xF3, 0x0F, 0x58, 0x89, 0xDC, 0x00, 0x00, 0x00, 
        0x0F, 0xB6, 0x81, 0xD1, 0x00, 0x00, 0x00
    };
    InstallInlineHook(base + RVA_TRAFFIC_LIGHT_TICK, (void*)Hook_TrafficLightTick,
                      (void**)&g_origTrafficLightTick, kExpect, sizeof(kExpect), "TrafficLightTick");
}

extern "C" __declspec(dllexport) unsigned TsmPluginApiVersion(void) { return TSM_API_VERSION; }

extern "C" __declspec(dllexport) int TsmPluginInit(const TsmHost* host, TsmPluginInfo* info)
{
    TsmBind(host);
    info->name    = "smart_traffic_lights";
    info->version = "1.0-smart";
    Logf("smart_traffic_lights: Init OK! Smart mode active.");
    return 0;
}

extern "C" __declspec(dllexport) int TsmPluginStart(void)
{
    ApplyPatches();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }
