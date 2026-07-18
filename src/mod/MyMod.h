#pragma once

#include <optional>
#include "mod/Config.h"
#include <pl/Mod.hpp>

// Include header Lua agar compiler mengenali tipe data lua_State
extern "C" {
    #include "lua.h"
}

namespace clange_me {

class ClangeMeMod {
  public:
    static ClangeMeMod &instance();

    ClangeMeMod();

    [[nodiscard]] ll::mod::NativeMod &getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

  private:
    ll::mod::NativeMod &mSelf;
    ModConfig mConfig;
    std::optional<pl::config::ConfigFile<ModConfig>> mConfigFile;
    
    // --- TAMBAHKAN INI ---
    lua_State* L; 
};

} // namespace clange_me
