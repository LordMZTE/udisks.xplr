#include "Manager.hxx"

#include <cassert>
#include <cstdlib>
#include <lua.hpp>
#include <new>
#include <thread>

constexpr const char *MANAGER_REG_KEY = "udisks_xplr_manager";

static std::thread *glib_loop_thread = nullptr;

static udisks_xplr::Manager *getOrCreateManager(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, MANAGER_REG_KEY);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        auto *manager_mem = lua_newuserdata(L, sizeof(udisks_xplr::Manager));
        auto manager = new (manager_mem) udisks_xplr::Manager();

        glib_loop_thread = new std::thread([manager]() { manager->gEventLoopThread(); });

        lua_createtable(L, 0, 1);
        lua_pushcfunction(L, [](lua_State *L) {
            luaL_checktype(L, 1, LUA_TUSERDATA);
            auto manager = static_cast<udisks_xplr::Manager *>(lua_touserdata(L, 1));
            manager->~Manager();
            glib_loop_thread->join();
            delete glib_loop_thread;
            return 0;
        });
        lua_setfield(L, -2, "__gc");

        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, MANAGER_REG_KEY);
    }

    auto ptr = lua_touserdata(L, -1);
    assert(ptr);
    lua_pop(L, 1);
    return static_cast<udisks_xplr::Manager *>(ptr);
}

extern "C" int luaopen_udisks_native(lua_State *L) {
    lua_createtable(L, 0, 2);

    lua_pushcfunction(L, [](lua_State *L) {
        auto mngr = getOrCreateManager(L);
        return mngr->luaRenderContentRows(L);
    });
    lua_setfield(L, -2, "renderContentRows");

    lua_pushcfunction(L, [](lua_State *L) {
        auto mngr = getOrCreateManager(L);
        return mngr->luaReload(L);
    });
    lua_setfield(L, -2, "reload");

    lua_pushcfunction(L, [](lua_State *L) {
        auto mngr = getOrCreateManager(L);
        return mngr->luaMoveCursor(L);
    });
    lua_setfield(L, -2, "moveCursor");

    lua_pushcfunction(L, [](lua_State *L) {
        auto mngr = getOrCreateManager(L);
        return mngr->luaMountSelected(L);
    });
    lua_setfield(L, -2, "mountSelected");

    lua_pushcfunction(L, [](lua_State *L) {
        auto mngr = getOrCreateManager(L);
        return mngr->luaUnmountSelected(L);
    });
    lua_setfield(L, -2, "unmountSelected");

    return 1;
}
