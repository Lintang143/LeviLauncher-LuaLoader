#include "mod/MyMod.h"
#include <filesystem>
#include <pl/Mod.hpp>          //[span_5](start_span)[span_5](end_span)
#include "pl/LuaBinding.hpp"  // Include file binding native kita

namespace loader {

ClangeMeMod &ClangeMeMod::instance() {
    static ClangeMeMod instance;
    return instance;
}

ClangeMeMod::ClangeMeMod() : mSelf(*ll::mod::NativeMod::current()), L(nullptr) {} //[span_6](start_span)[span_6](end_span)

bool ClangeMeMod::load() {
    auto &self = getSelf(); //[span_7](start_span)[span_7](end_span)
    self.getLogger().debug("Loading..."); //[span_8](start_span)[span_8](end_span)

    std::error_code ec;
    std::filesystem::create_directories(self.getDataDir(), ec); //[span_9](start_span)[span_9](end_span)
    if (ec) {
        self.getLogger().error("Failed to create data directory {}: {}", self.getDataDir().string(), ec.message()); //[span_10](start_span)[span_10](end_span)
        return false;
    }
    
    std::filesystem::create_directories(self.getConfigDir(), ec); //[span_11](start_span)[span_11](end_span)
    if (ec) {
        self.getLogger().error("Failed to create config directory {}: {}", self.getConfigDir().string(), ec.message()); //[span_12](start_span)[span_12](end_span)
        return false;
    }

    self.getLogger().info("[Logger] Mod Main Folder: {}", self.getModDir().string()); //[span_13](start_span)[span_13](end_span)
    self.getLogger().info("[Logger] Mod Data Folder: {}", self.getDataDir().string()); //[span_14](start_span)[span_14](end_span)
    self.getLogger().info("[Logger] Mod Config Folder: {}", self.getConfigDir().string()); //[span_15](start_span)[span_15](end_span)

    std::filesystem::path modsPath = self.getDataDir() / "mods"; //[span_16](start_span)[span_16](end_span)
    
    if (std::filesystem::exists(modsPath)) { //[span_17](start_span)[span_17](end_span)
        self.getLogger().info("[INFO] 'mods' folder already exist in: {}. cannot create folder", modsPath.string()); //[span_18](start_span)[span_18](end_span)
    } else {
        self.getLogger().info("'mods' folder not exist. Creating folder in: {}", modsPath.string()); //[span_19](start_span)[span_19](end_span)
        std::filesystem::create_directories(modsPath, ec); //[span_20](start_span)[span_20](end_span)
        if (ec) {
            self.getLogger().error("failed while making 'mods' folder:{}", ec.message()); //[span_21](start_span)[span_21](end_span)
        } else {
            self.getLogger().info("[SUKSES] Folder 'mods' berhasil dibuat untuk pertama kali!"); //[span_22](start_span)[span_22](end_span)
        }
    }

    mConfigFile.emplace(); //[span_23](start_span)[span_23](end_span)
    if (!mConfigFile->load()) { //[span_24](start_span)[span_24](end_span)
        self.getLogger().warn("Failed to load typed config"); //[span_25](start_span)[span_25](end_span)
        return false;
    }
    mConfig = mConfigFile->value(); //[span_26](start_span)[span_26](end_span)

    self.getLogger().info("Loaded {} from {}", self.getName(), self.getModDir().string()); //[span_27](start_span)[span_27](end_span)
    return true;
}

bool ClangeMeMod::enable() {
    auto &self = getSelf(); //[span_28](start_span)[span_28](end_span)
    self.getLogger().debug("Enabling..."); //[span_29](start_span)[span_29](end_span)
    
    if (!mConfig.enabled) { //[span_30](start_span)[span_30](end_span)
        self.getLogger().info("clange_me is disabled by config"); //[span_31](start_span)[span_31](end_span)
        return true;
    }

    self.getLogger().info("Config message: {}", mConfig.message); //[span_32](start_span)[span_32](end_span)

    // --- INISIALISASI & JALANKAN LUA DI SINI ---
    self.getLogger().info("[Lua Engine] Menginisialisasi Lua State...");
    L = luaL_newstate();
    if (!L) {
        self.getLogger().error("[Lua Engine] Gagal membuat Lua State!");
        return false;
    }

    luaL_openlibs(L);
    pl::lua_binding::register_all_pl_modules(L);

    std::filesystem::path modsPath = self.getDataDir() / "mods"; //[span_33](start_span)[span_33](end_span)
    std::error_code ec;

    if (std::filesystem::exists(modsPath)) {
        self.getLogger().info("[Lua Engine] Memindai berkas script Lua di: {}", modsPath.string());
        
        for (const auto& entry : std::filesystem::directory_iterator(modsPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lua") {
                self.getLogger().info("[Lua Engine] Mengeksekusi script: {}", entry.path().filename().string());
                
                if (luaL_dofile(L, entry.path().string().c_str()) != LUA_OK) {
                    self.getLogger().error("[Lua Error] Gagal memuat {}: {}", 
                                           entry.path().filename().string(), lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
        }
    }

    // Picu event 'enable' yang didaftarkan di dalam berkas script Lua
    self.getLogger().info("[Lua Engine] Memicu event enable script...");
    pl::lua_binding::triggerLuaLifecycle(L, pl::lua_binding::luaEnableRef);

    return true;
}

bool ClangeMeMod::disable() {
    auto &self = getSelf(); //[span_34](start_span)[span_34](end_span)
    self.getLogger().debug("Disabling..."); //[span_35](start_span)[span_35](end_span)

    if (L) {
        self.getLogger().info("[Lua Engine] Memicu event disable script...");
        // Picu event 'disable' di script Lua
        pl::lua_binding::triggerLuaLifecycle(L, pl::lua_binding::luaDisableRef);
        
        self.getLogger().info("[Lua Engine] Menutup Lua State...");
        lua_close(L);
        L = nullptr;
    }
    return true;
}

bool ClangeMeMod::unload() {
    auto &self = getSelf(); //[span_36](start_span)[span_36](end_span)
    self.getLogger().debug("Unloading..."); //[span_37](start_span)[span_37](end_span)
    
    // Log sederhana menandakan Lua telah berhenti dijalankan total
    self.getLogger().info("[Lua Engine] Lua subsystem has been completely unloaded.");

    mConfigFile.reset(); //[span_38](start_span)[span_38](end_span)
    return true;
}

} // namespace loader
