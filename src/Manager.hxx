#pragma once

#include "udisks.hxx"
#include <lua.hpp>
#include <mutex>
#include <udisks/udisks-generated.h>
#include <udisks/udisks.h>

namespace udisks_xplr {
class Manager {
public:
    Manager();

    ~Manager();

    void gEventLoopThread();

    int luaRenderContentRows(lua_State *L);
    int luaReload(lua_State *L);
    int luaMoveCursor(lua_State *L);
    int luaMountSelected(lua_State *L);
    int luaUnmountSelected(lua_State *L);

private:
    void postMessage(std::string str);
    struct udisks::DeviceInfo *getMaybeDeviceUnderCursor();

    GMainLoop *m_loop;
    std::mutex m_mtx;
    std::string m_error;
    std::string m_message;

    UDisksClient *m_client{};
    UDisksManager *m_mngr{};
    std::vector<struct udisks::DeviceInfo> m_devices{};
    uint m_cursor{ 0 };
};
} //namespace udisks_xplr
