#include "zygisk.hpp"
#include <cstdio>

#define LOGI(...) fprintf(stderr, "[WeaiText] " __VA_ARGS__); fprintf(stderr, "\n")

class WeaiMod : public zygisk::Module {
public:
    void onLoad(zygisk::API* api) override { 
        LOGI("极简版：模块加载成功！");
    }
};

REGISTER_ZYGISK_MODULE(WeaiMod)
