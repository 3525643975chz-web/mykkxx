#pragma once

#include <jni.h>

namespace zygisk {
    struct AppSpecializeArgs {
        jint *uid;
        jint *gid;
        jint *gids;
        jint gids_size;
        jint *fds_to_close;
        jint fds_to_close_length;
        jint *fds_to_ignore;
        jint fds_to_ignore_length;
        jboolean *is_child_zygote;
        jboolean *is_top_app;
        jobjectArray *pkg_data_info_list;
        jobjectArray *whitelisted_data_info_list;
        jboolean *mount_data_dirs;
        jboolean *mount_storage_dirs;
        jstring *nice_name;
        jstring *instruction_set;
        jstring *app_data_dir;
    };

    enum class Option : int {
        DLCLOSE_MODULE_LIBRARY = 1 << 0,
    };

    class Api;
    class ModuleBase {
    public:
        virtual void onLoad(Api* api, JNIEnv* env) {}
        virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    };
}
