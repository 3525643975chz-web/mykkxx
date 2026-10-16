// Zygisk API v4 header (public, no external system deps)
#pragma once

#include <jni.h>
#include <stdint.h>

namespace zygisk {

struct Api;
struct Module;

using ApiTable = struct Api {
    uint32_t version;
    void (*hook_jni)(JNIEnv *env, const char *className, const JNINativeMethod *methods, size_t count);
    void (*plt_hook_register)(void *symbol, void *newFunc, void **oldFunc);
    void (*plt_hook_commit)();
    void (*set_option)(uint64_t option);
    int (*connect_companion)();
    int (*get_module_dir)();
    int (*get_flags)();
    void (*set_log_tag)(const char *tag);
    void (*set_env)();
};

enum Option : uint64_t {
    FORCE_DENYLIST_UNMOUNT = 1u << 0,
};

enum StateFlag : uint32_t {
    PROCESS_GRANTED_ROOT = 1u << 0,
    PROCESS_ON_DENYLIST = 1u << 1,
};

struct Module {
    Api *api;
    virtual void onLoad(Api *api) = 0;
    virtual void preAppSpecialize(const char *niceName) = 0;
    virtual void postAppSpecialize(const char *niceName) = 0;
    virtual void postServerSpecialize() {}
    virtual ~Module() = default;
};

#define REGISTER_ZYGISK_MODULE(mod) \
    extern "C" [[gnu::visibility("default")]] void zygisk_module_entry(zygisk::Api *api, zygisk::Module **out) { \
        static mod instance; \
        instance.onLoad(api); \
        *out = &instance; \
    }

} // namespace zygisk
