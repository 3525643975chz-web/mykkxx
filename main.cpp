extern "C" jint JNI_GetCreatedJavaVMs(JavaVM **vmBuf, jsize bufLen, jsize *nVMs);
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

struct Rule { std::string from, to; };
static bool g_enabled = false;
static std::vector<std::string> g_targets;
static std::vector<Rule> g_rules;
static bool g_isTarget = false;

static const char* CFG = "/data/adb/modules/WeaiText/config.json";

static std::string replaceAll(std::string s, const std::string& f, const std::string& t) {
    if (f.empty()) return s;
    size_t p = 0;
    while ((p = s.find(f, p)) != std::string::npos) { s.replace(p, f.size(), t); p += t.size(); }
    return s;
}

static void loadCfg() {
    g_enabled = false; g_targets.clear(); g_rules.clear();
    std::ifstream f(CFG);
    if (!f) { LOGE("配置文件打不开"); return; }
    std::stringstream ss; ss << f.rdbuf(); std::string raw = ss.str(); f.close();
    
    // 简单粗暴的 JSON 解析
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
    
    size_t rs = raw.find("\"rules\"");
    if (rs != std::string::npos) {
        size_t p = rs;
        while (true) {
            size_t fk = raw.find("\"from\"", p); if (fk == std::string::npos) break;
            size_t fq1 = raw.find('"', fk+6); if (fq1 == std::string::npos) break;
            size_t fq2 = raw.find('"', fq1+1); if (fq2 == std::string::npos) break;
            std::string from = raw.substr(fq1+1, fq2-fq1-1);
            
            size_t tk = raw.find("\"to\"", fq2); if (tk == std::string::npos) break;
            size_t tq1 = raw.find('"', tk+4); if (tq1 == std::string::npos) break;
            size_t tq2 = raw.find('"', tq1+1); if (tq2 == std::string::npos) break;
            std::string to = raw.substr(tq1+1, tq2-tq1-1);
            
            g_rules.push_back({from, to});
            p = tq2;
        }
    }
    LOGI("配置: 开关%d 目标%zu 规则%zu", g_enabled, g_targets.size(), g_rules.size());
}

static bool pkgMatch(JNIEnv* env) {
    jclass at = env->FindClass("android/app/ActivityThread");
    if (!at) return false;
    
    jmethodID getCurrentActivityThread = env->GetStaticMethodID(at, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject atObj = env->CallStaticObjectMethod(at, getCurrentActivityThread);
    if (!atObj) { env->DeleteLocalRef(at); return false; }
    
    jmethodID getPkg = env->GetMethodID(at, "getPackageName", "()Ljava/lang/String;");
    jstring pj = (jstring)env->CallObjectMethod(atObj, getPkg);
    if (!pj) { env->DeleteLocalRef(atObj); env->DeleteLocalRef(at); return false; }
    
    const char* pkg = env->GetStringUTFChars(pj, nullptr);
    if (!pkg) { env->DeleteLocalRef(pj); env->DeleteLocalRef(atObj); env->DeleteLocalRef(at); return false; }
    
    std::string currentPkg(pkg);
    env->ReleaseStringUTFChars(pj, pkg);
    
    env->DeleteLocalRef(pj);
    env->DeleteLocalRef(atObj);
    env->DeleteLocalRef(at);
    
    for (auto& t : g_targets) if (currentPkg == t) return true;
    return false;
}

static jstring repl(JNIEnv* env, jstring in) {
    if (!g_enabled || g_rules.empty() || !in) return in;
    const char* raw = env->GetStringUTFChars(in, nullptr);
    if (!raw) return in;
    std::string text(raw); 
    env->ReleaseStringUTFChars(in, raw);
    for (auto& r : g_rules) text = replaceAll(text, r.from, r.to);
    return env->NewStringUTF(text.c_str());
}

static jboolean (*orig_commit)(JNIEnv*, jobject, jobject, jint) = nullptr;
static jboolean (*orig_setcomp)(JNIEnv*, jobject, jobject, jint) = nullptr;

static jboolean h_commit(JNIEnv* env, jobject thiz, jobject text, jint cursor) {
    if (g_isTarget && text) {
        jclass cls = env->GetObjectClass(text);
        jmethodID ts = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
        jstring s = (jstring)env->CallObjectMethod(text, ts);
        jstring ns = repl(env, s);
        jboolean result = orig_commit(env, thiz, ns, cursor);
        env->DeleteLocalRef(cls);
        if (s) env->DeleteLocalRef(s);
        if (ns && ns != s) env->DeleteLocalRef(ns);
        return result;
    }
    return orig_commit(env, thiz, text, cursor);
}

static jboolean h_setcomp(JNIEnv* env, jobject thiz, jobject text, jint cursor) {
    if (g_isTarget && text) {
        jclass cls = env->GetObjectClass(text);
        jmethodID ts = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
        jstring s = (jstring)env->CallObjectMethod(text, ts);
        jstring ns = repl(env, s);
        jboolean result = orig_setcomp(env, thiz, ns, cursor);
        env->DeleteLocalRef(cls);
        if (s) env->DeleteLocalRef(s);
        if (ns && ns != s) env->DeleteLocalRef(ns);
        return result;
    }
    return orig_setcomp(env, thiz, text, cursor);
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
        LOGI("命中目标 %s，安装Hook", n);

        jclass ic = env->FindClass("android/view/inputmethod/InputConnection");
        JNINativeMethod m[] = {
            {(char*)"commitText", (char*)"(Ljava/lang/CharSequence;I)Z", (void*)h_commit},
            {(char*)"setComposingText", (char*)"(Ljava/lang/CharSequence;I)Z", (void*)h_setcomp}
        };
        api->hook_jni(env, "android/view/inputmethod/InputConnection", m, 2);
    }
    // 这里原来有一行 serverSpecialize()，已经删掉了
};

REGISTER_ZYGISK_MODULE(WeaiMod)
