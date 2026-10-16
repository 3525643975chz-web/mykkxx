#include "zygisk.hpp"
#include <cstdio>

#define LOGI(...) fprintf(stderr, "[WeaiText] " __VA_ARGS__); fprintf(stderr, "\n")

class WeaiMod : public zygisk::Module {
public:
    void onLoad(zygisk::Api* api) override { 
        LOGI("极简版：模块加载成功！"); 
    }
    void preAppSpecialize(const char* niceName) override {}
    void postAppSpecialize(const char* niceName) override {}
    void postServerSpecialize() override {}
};

REGISTER_ZYGISK_MODULE(WeaiMod)
