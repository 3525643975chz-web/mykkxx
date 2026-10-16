#include "zygisk.hpp"
#include <stdio.h>

class WeaiMod : public zygisk::Module {
public:
    void onLoad(zygisk::Api* api) override { 
        fprintf(stderr, "[WeaiText] 纯血版：模块加载成功！\n"); 
    }
    void preAppSpecialize(const char* niceName) override {}
    void postAppSpecialize(const char* niceName) override {}
    void postServerSpecialize() override {}
};

REGISTER_ZYGISK_MODULE(WeaiMod)
