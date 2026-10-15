#include <jni.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;

static const char* MOD_PATH = "/data/adb/modules/meovratext";
static const char* RULES_FILE = "/data/adb/modules/meovratext/rules.json";
static const char* TARGET_FILE = "/data/adb/modules/meovratext/targets.json";

static std::unordered_map<std::string, std::string> g_rules;
static bool g_active = false;
static JNIEnv* g_env = nullptr;
static jclass g_tvClazz = nullptr;
static jmethodID g_origSetText = nullptr;

static std::string apply(const std::string& in) {
    if (in.empty() || g_rules.empty()) return in;
    std::string out = in;
    for (auto& [k, v] : g_rules) {
        if (k.empty()) continue;
        size_t pos = 0;
        while ((pos = out.find(k, pos)) != std::string::npos) {
            out.replace(pos, k.size(), v);
            pos += v.size();
        }
    }
    return out;
}

static void hooked_setText(JNIEnv* env, jobject thiz, jstring text) {
    if (g_active && text) {
        const char* raw = env->GetStringUTFChars(text, nullptr);
        std::string replaced = apply(raw ? raw : "");
        env->ReleaseStringUTFChars(text, raw);
        jstring newStr = env->NewStringUTFChars(env, replaced.c_str()); // 注意：这里用 NewStringUTF 也可以，但更安全是用 NewStringUTF 或 NewString
        // 简单处理：直接用 NewStringUTF
        jstring safeStr = env->NewStringUTF(replaced.c_str());
        env->CallNonvirtualVoidMethod(thiz, g_tvClazz, g_origSetText, safeStr);
        env->DeleteLocalRef(safeStr);
        return;
    }
    env->CallNonvirtualVoidMethod(thiz, g_tvClazz, g_origSetText, text);
}

static void do_hook(JNIEnv* env) {
    g_env = env;
    jclass c = env->FindClass("android/widget/TextView");
    if (!c) return;
    g_tvClazz = (jclass) env->NewGlobalRef(c);
    JNINativeMethod m{"setText", "(Ljava/lang/CharSequence;)V", (void*)hooked_setText};
    env->RegisterNatives(g_tvClazz, &m, 1);
    env->DeleteLocalRef(c);
}

class Meovra : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        do_hook(env);
    }
    void postAppSpecialize(const AppSpecializeArgs* args) override {
        if (!args || !args->nice_name) { g_active = false; return; }
        const char* pkg = g_env->GetStringUTFChars(args->nice_name, nullptr);
        std::string name = pkg ? pkg : "";
        g_env->ReleaseStringUTFChars(args->nice_name, pkg);

        // 跳过系统核心进程，防止死机
        if (name == "android" || name == "com.android.systemui") { g_active = false; return; }

        // 读 targets.json 判断是否在列表里
        bool is_target = false;
        std::ifstream in_t(TARGET_FILE);
        if (in_t.is_open()) {
            std::stringstream buf; buf << in_t.rdbuf();
            std::string all = buf.str();
            in_t.close();
            is_target = (all.find("\"" + name + "\"") != std::string::npos);
        }
        if (!is_target) { g_active = false; return; }

        // 读 rules.json
        g_rules.clear();
        std::ifstream in_r(RULES_FILE);
        if (in_r.is_open()) {
            std::stringstream buf; buf << in_r.rdbuf();
            std::string all = buf.str();
            in_r.close();
            size_t p = 0;
            while ((p = all.find("{", p)) != std::string::npos) {
                size_t e = all.find("}", p); if (e == std::string::npos) break;
                std::string obj = all.substr(p, e - p + 1);
                std::string from, to;
                auto pick = [&](const char* key, std::string& out) {
                    std::string k = std::string("\"") + key + "\"";
                    size_t a = obj.find(k); if (a == std::string::npos) return false;
                    a = obj.find(':', a + k.size()); if (a == std::string::npos) return false;
                    a = obj.find('"', a); if (a == std::string::npos) return false;
                    size_t b = obj.find('"', a + 1); if (b == std::string::npos) return false;
                    out = obj.substr(a + 1, b - a - 1);
                    return true;
                };
                if (pick("from", from) && pick("to", to) && !from.empty()) g_rules[from] = to;
                p = e + 1;
            }
        }
        g_active = !g_rules.empty();
    }
};

REGISTER_ZYGISK_MODULE(Meovra)
