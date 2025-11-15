#include "udisks.hxx"
#include <gio/gdbusinterface.h>
#include <gio/gdbusobject.h>
#include <sstream>
#include <udisks/udisks-generated.h>

void udisks_xplr::udisks::fillDevices(
    UDisksClient *client, std::vector<struct DeviceInfo> &devices
) {
    auto objects = g_dbus_object_manager_get_objects(udisks_client_get_object_manager(client));

    for (auto ob = objects; ob != nullptr; ob = ob->next) {
        auto udisks_obj = UDISKS_OBJECT(ob->data);
        auto drive = udisks_object_peek_drive(udisks_obj);
        if (!drive)
            continue;

        UDisksFilesystem *maybe_fs = nullptr;
        std::string model{ udisks_drive_get_model(drive) };
        std::stringstream block;
        std::stringstream mounts;

        auto objpath = g_dbus_object_get_object_path(G_DBUS_OBJECT(ob->data));
        for (auto maybe_block = objects; maybe_block != nullptr; maybe_block = maybe_block->next) {
            auto blk = udisks_object_get_block(UDISKS_OBJECT(maybe_block->data));
            if (!blk || strcmp(objpath, udisks_block_get_drive(blk)) != 0)
                continue;

            if (block.rdbuf()->in_avail() != 0)
                block << ", ";
            block << udisks_block_get_device(blk);

            auto fs = udisks_object_peek_filesystem(UDISKS_OBJECT(maybe_block->data));
            if (fs) {
                maybe_fs = fs;
                auto mountpoints = udisks_filesystem_get_mount_points(fs);
                while (*mountpoints) {
                    if (mounts.rdbuf()->in_avail() != 0)
                        mounts << ", ";
                    mounts << *mountpoints++;
                }
            }
        }

        devices.push_back(
            { .maybe_fs = maybe_fs,
              .model = std::move(model),
              .block = block.str(),
              .mounts = mounts.str() }
        );
    }
}
