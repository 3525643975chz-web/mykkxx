#include "zygisk.hpp"
#include <cstdio>

#define LOGI(...) fprintf(stderr, "[WeaiText] " __VA_ARGS__); fprintf(stderr, "\n")

class WeaiMod : public zygisk::Module {
public:
    // 修复拼写：必须叫 zygisk::Api，不是 API
    void onLoad(zygisk::Api* api) override { 
        LOGI("极简版：模块加载成功！"); 
    }
    
    // 补全抽象类要求的其他接口，留空即可，防止编译器报错
    void preAppSpecialize(const char* niceName) override {}
    void postAppSpecialize(const char* niceName) override {}
    void postServerSpecialize() override {}

};

REGISTER_ZYGISK_MODULE(WeaiMod)
