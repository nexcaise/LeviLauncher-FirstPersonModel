#include <jni.h>
#include <android/log.h>
#include "pl/internal/Macro.h"

#define LOG_TAG "HmmAja"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

PLCAPI void LeviMod_LoadBefore() {
    LOGI("LeviMod_LoadBefore");
}

PLCAPI void LeviMod_LoadAfter() {
    LOGI("LeviMod_LoadAfter");
}

__attribute__((constructor))
static void Init() {
    LOGI("Init");
}