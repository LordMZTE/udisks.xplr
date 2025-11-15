#pragma once

#include <string>
#include <udisks/udisks.h>
#include <vector>

namespace udisks_xplr::udisks {
struct DeviceInfo {
    UDisksFilesystem *maybe_fs;
    std::string model;
    std::string block;
    std::string mounts;
};

void fillDevices(UDisksClient *client, std::vector<struct DeviceInfo> &devices);
} //namespace udisks_xplr::udisks
