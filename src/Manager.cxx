#include "Manager.hxx"
#include "util.hxx"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <lauxlib.h>
#include <lua.h>
#include <mutex>
#include <udisks/udisks-generated.h>
#include <udisks/udisks.h>

udisks_xplr::Manager::Manager(): m_loop(g_main_loop_new(nullptr, false)) {}

udisks_xplr::Manager::~Manager() {
    g_main_loop_quit(m_loop);
    if (m_client) {
        g_object_unref(m_client);
    }

    // m_mngr is intentionally not freed here as that is owned by m_client.
}

void udisks_xplr::Manager::gEventLoopThread() {
    udisks_client_new(nullptr, [](GObject *src, GAsyncResult *res, gpointer mngr) {
        GError *err{};
        auto client = udisks_client_new_finish(res, &err);

        auto self = reinterpret_cast<udisks_xplr::Manager *>(mngr);
        std::lock_guard<std::mutex> guard(self->m_mtx);
        if (err) {
            self->m_error = err->message;
        } else {
            self->m_client = client;
            self->m_mngr = udisks_client_get_manager(self->m_client);

            auto on_objects_changed = reinterpret_cast<GCallback>(
                +[](GDBusObjectManager *mngr, GDBusObject *_, gpointer udata) {
                auto self = reinterpret_cast<udisks_xplr::Manager *>(udata);
                std::lock_guard<std::mutex> guard(self->m_mtx);
                self->m_devices.clear();
                udisks_xplr::util::redrawXplr();
            }
            );

            auto obj_mngr = udisks_client_get_object_manager(self->m_client);
            g_signal_connect(obj_mngr, "object-added", on_objects_changed, self);
            g_signal_connect(obj_mngr, "object-removed", on_objects_changed, self);
        }
        udisks_xplr::util::redrawXplr();
    }, this);

    g_main_loop_run(m_loop);
}

void udisks_xplr::Manager::postMessage(std::string str) {
    m_message = std::move(str);
    udisks_xplr::util::redrawXplr();

    g_timeout_add_once(2000, [](gpointer udata) {
        auto self = static_cast<udisks_xplr::Manager *>(udata);
        std::lock_guard<std::mutex> guard(self->m_mtx);
        self->m_message.clear();
        udisks_xplr::util::redrawXplr();
    }, this);
}

int udisks_xplr::Manager::luaRenderContentRows(lua_State *L) {
    std::lock_guard<std::mutex> guard(m_mtx);
    if (!m_error.empty()) {
        lua_pushstring(L, "CustomParagraph");
        lua_pushstring(L, "Error: ");
        lua_pushlstring(L, m_error.c_str(), m_error.size());
        lua_pushstring(L, "\nReload to clear error.");
        lua_concat(L, 3);
        return 2;
    }

    if (!m_message.empty()) {
        lua_pushstring(L, "CustomParagraph");
        lua_pushlstring(L, m_message.c_str(), m_message.size());
        return 2;
    }

    if (!m_client) {
        lua_pushstring(L, "CustomParagraph");
        lua_pushstring(L, "Loading...");
        return 2;
    }

    if (!m_mngr) {
        // The manager being null indicates that udisks isn't running.
        lua_pushstring(L, "CustomParagraph");
        lua_pushstring(L, "UDisks is not running.");
        return 2;
    }

    if (m_devices.empty()) {
        udisks::fillDevices(m_client, m_devices);
    }

    lua_pushstring(L, "CustomTable");
    lua_newtable(L);

    lua_newtable(L);
    lua_pushstring(L, ""); // cursor
    lua_rawseti(L, -2, 1);
    lua_pushstring(L, "model");
    lua_rawseti(L, -2, 2);
    lua_pushstring(L, "block");
    lua_rawseti(L, -2, 3);
    lua_pushstring(L, "mounts");
    lua_rawseti(L, -2, 4);

    lua_rawseti(L, -2, 1);

    int i{ 2 };
    for (const auto &dev : m_devices) {
        lua_createtable(L, 2, 0);

        lua_pushstring(L, m_cursor == i - 2 ? "> " : "  ");
        lua_rawseti(L, -2, 1);

        lua_pushlstring(L, dev.model.c_str(), dev.model.size());
        lua_rawseti(L, -2, 2);

        lua_pushlstring(L, dev.block.c_str(), dev.block.size());
        lua_rawseti(L, -2, 3);

        lua_pushlstring(L, dev.mounts.c_str(), dev.mounts.size());
        lua_rawseti(L, -2, 4);

        lua_rawseti(L, -2, i++);
    }

    lua_createtable(L, 2, 0);

    lua_createtable(L, 0, 1);
    lua_pushnumber(L, 2);
    lua_setfield(L, -2, "Length");
    lua_rawseti(L, -2, 1);

    for (i = 2; i <= 4; i++) {
        lua_createtable(L, 0, 1);
        lua_pushinteger(L, 33);
        lua_setfield(L, -2, "Percentage");
        lua_rawseti(L, -2, i);
    }

    return 3;
}

int udisks_xplr::Manager::luaReload(lua_State *L) {
    std::lock_guard<std::mutex> guard(m_mtx);
    // TODO: This isn't entirely correct. If the error happened during client
    // initialization, we won't try again and this will just result in us showing "Loading..."
    m_error.clear();
    m_devices.clear();
    m_cursor = 0;
    return 0;
}

int udisks_xplr::Manager::luaMoveCursor(lua_State *L) {
    std::lock_guard<std::mutex> guard(m_mtx);
    auto n = luaL_checkinteger(L, 1);
    if (m_devices.empty()) {
        m_cursor = 0;
    } else if (m_cursor == 0 && n < 0) {
        m_cursor = m_devices.size() - 1;
    } else {
        m_cursor = (m_cursor + n) % m_devices.size();
    }
    return 0;
}

struct udisks_xplr::udisks::DeviceInfo *udisks_xplr::Manager::getMaybeDeviceUnderCursor() {
    if (m_cursor >= m_devices.size())
        return nullptr;
    return &m_devices[m_cursor];
}

constexpr const char *NO_FS_MSG = "The selected device has no filesystem.";

int udisks_xplr::Manager::luaMountSelected(lua_State *L) {
    std::lock_guard<std::mutex> guard(m_mtx);
    auto dev = getMaybeDeviceUnderCursor();
    if (!dev || !dev->maybe_fs) {
        postMessage(NO_FS_MSG);
        return 0;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    auto options = g_variant_builder_end(&builder);
    g_variant_ref_sink(options);
    m_message = "Mounting " + dev->model + "...";
    udisks_filesystem_call_mount(
        dev->maybe_fs, options, nullptr, [](GObject *src, GAsyncResult *res, gpointer mngr) {
        auto self = static_cast<udisks_xplr::Manager *>(mngr);
        std::lock_guard<std::mutex> guard(self->m_mtx);
        GError *err = nullptr;
        udisks_filesystem_call_mount_finish(UDISKS_FILESYSTEM(src), nullptr, res, &err);
        self->m_devices.clear();
        self->m_message.clear();
        if (err) {
            self->m_error = err->message;
        }
        udisks_xplr::util::redrawXplr();
    }, this
    );

    return 0;
}

int udisks_xplr::Manager::luaUnmountSelected(lua_State *L) {
    std::lock_guard<std::mutex> guard(m_mtx);
    auto dev = getMaybeDeviceUnderCursor();
    if (!dev || !dev->maybe_fs) {
        postMessage(NO_FS_MSG);
        return 0;
    }

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    auto options = g_variant_builder_end(&builder);
    g_variant_ref_sink(options);
    m_message = "Unmounting " + dev->model + "...";
    udisks_filesystem_call_unmount(
        dev->maybe_fs, options, nullptr, [](GObject *src, GAsyncResult *res, gpointer mngr) {
        auto self = static_cast<udisks_xplr::Manager *>(mngr);
        std::lock_guard<std::mutex> guard(self->m_mtx);
        GError *err = nullptr;
        udisks_filesystem_call_unmount_finish(UDISKS_FILESYSTEM(src), res, &err);
        self->m_devices.clear();
        self->m_message.clear();
        if (err) {
            self->m_error = err->message;
        }
        udisks_xplr::util::redrawXplr();
    }, this
    );

    return 0;
}
