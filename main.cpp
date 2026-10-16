#include "zygisk.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <jni.h>
#include <cstring>
#include <cstdio>

#define LOGI(...) fprintf(stderr, "[WeaiText] " __VA_ARGS__); fprintf(stderr, "\n")
#define LOGE(...) fprintf(stderr, "[WeaiText-ERR] " __VA_ARGS__); fprintf(stderr, "\n")

static bool g_enabled = false;
static std::vector<std::string> g_targets;
static bool g_isTarget = false;

static const char* CFG = "/data/adb/modules/WeaiText/config.json";

static void loadCfg() {
    g_enabled = false; g_targets.clear();
    std::ifstream f(CFG);
    if (!f) { LOGE("配置文件打不开"); return; }
    std::stringstream ss; ss << f.rdbuf(); std::string raw = ss.str(); f.close();
    g_enabled = raw.find("\"enabled\":true") != std::string::npos || raw.find("\"enabled\": true") != std::string::npos;
    size_t ts = raw.find("\"targets\"");
    if (ts != std::string::npos) {
        size_t a = raw.find('[', ts), b = raw.find(']', a);
        if (a != std::string::npos && b != std::string::npos) {
            std::string arr = raw.substr(a+1, b-a-1);
            size_t i = 0;
            while (i < arr.size()) {
                size_t q1 = arr.find('"', i); if (q1 == std::string::npos) break;
                size_t q2 = arr.find('"', q1+1); if (q2 == std::string::npos) break;
                g_targets.push_back(arr.substr(q1+1, q2-q1-1));
                i = q2+1;
            }
        }
    }
    LOGI("配置加载: 开关%d 目标数量%zu", g_enabled, g_targets.size());
}

static bool pkgMatch(JNIEnv* env) {
    jclass at = env->FindClass("android/app/ActivityThread");
    if (!at) return false;
    jmethodID cur = env->GetStaticMethodID(at, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject atObj = env->CallStaticObjectMethod(at, cur);
    jmethodID gp = env->GetMethodID(at, "getPackageName", "()Ljava/lang/String;");
    jstring pj = (jstring)env->CallObjectMethod(atObj, gp);
    const char* pkg = env->GetStringUTFChars(pj, nullptr);
    std::string currentPkg(pkg);
    env->ReleaseStringUTFChars(pj, pkg);
    env->DeleteLocalRef(pj); env->DeleteLocalRef(atObj); env->DeleteLocalRef(at);
    for (auto& t : g_targets) if (currentPkg == t) return true;
    return false;
}

static jboolean (*orig_commit)(JNIEnv*, jobject, jobject, jint) = nullptr;

static jboolean h_commit(JNIEnv* env, jobject thiz, jobject text, jint cursor) {
    if (g_isTarget && text) {
        LOGI("安全Hook触发！成功拦截到输入，未修改任何内容！");
    }
    return orig_commit(env, thiz, text, cursor);
}

class WeaiMod : public zygisk::Module {
public:
    void onLoad(zygisk::Api* api) override { this->api = api; }
    void preAppSpecialize(const char* n) override { loadCfg(); }

    void postAppSpecialize(const char* n) override {
        if (!g_enabled) return;
        JavaVM* vm = nullptr; JNIEnv* env = nullptr;
        JNI_GetCreatedJavaVMs(&vm, 1, nullptr);
        if (!vm) return;
        vm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (!env) vm->AttachCurrentThread(&env, nullptr);
        if (!env) return;
        g_isTarget = pkgMatch(env);
        if (!g_isTarget) return;
        LOGI("命中目标 %s，安装安全Hook", n);
        jclass ic = env->FindClass("android/view/inputmethod/InputConnection");
        JNINativeMethod m[] = {
            {(char*)"commitText", (char*)"(Ljava/lang/CharSequence;I)Z", (void*)h_commit}
        };
        api->hook_jni(env, "android/view/inputmethod/InputConnection", m, 1);
    }
    void postServerSpecialize() override {}
};

REGISTER_ZYGISK_MODULE(WeaiMod)
