#include <cstdint>
#include <dlfcn.h>

#include "pl/Signature.h"
#include "pl/Hook.h"
#include "pl/Gloss.h"

constexpr uintptr_t CAMERA_POS_OFFSET = 0x18;

static float g_CameraOffsetX = 0.0f;
static float g_CameraOffsetY = -0.2f;
static float g_CameraOffsetZ = 0.5f;

static inline float* CameraPos(void* camera) {
    return reinterpret_cast<float*>(
        reinterpret_cast<uintptr_t>(camera) + CAMERA_POS_OFFSET
    );
}

static inline void CameraSetPosition(void* camera, float x, float y, float z) {
    if (!camera) return;
    float* pos = CameraPos(camera);
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
}

static void* (*orig_CameraRender)(void*, float) = nullptr;

static void* hook_CameraRender(void* camera, float deltaTime) {
    if (camera) {
        float* pos = CameraPos(camera);

        float ox = pos[0];
        float oy = pos[1];
        float oz = pos[2];

        pos[0] = ox + g_CameraOffsetX;
        pos[1] = oy + g_CameraOffsetY;
        pos[2] = oz + g_CameraOffsetZ;

        void* ret = orig_CameraRender
            ? orig_CameraRender(camera, deltaTime)
            : nullptr;

        pos[0] = ox;
        pos[1] = oy;
        pos[2] = oz;

        return ret;
    }

    return orig_CameraRender
        ? orig_CameraRender(camera, deltaTime)
        : nullptr;
}

static inline bool Hook(const char* sig, void* hook, void** orig) {
    uintptr_t addr = pl::signature::pl_resolve_signature(sig, "libminecraftpe.so");
    if (!addr) return false;

    return pl::hook::pl_hook(
        (pl::hook::FuncPtr)addr,
        (pl::hook::FuncPtr)hook,
        (pl::hook::FuncPtr*)orig,
        pl::hook::PriorityHighest
    ) == 0;
}

__attribute__((constructor))
static void FirstPersonModel_Init() {
    GlossInit(true);
    Hook(
        "48 ?? ?? ?? 21 ?? ?? 91 ?? ?? 00 ?? ?? 91 ?? ?? 40 ?? ?? 00 00 ?? ?? 00 94",
        (void*)hook_CameraRender,
        (void**)&orig_CameraRender
    );
}