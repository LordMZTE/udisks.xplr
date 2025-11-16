local M = {}

function M.setup(opts)
    if type(opts.install_path) ~= "boolean" or opts.install_path then
        local install_path = opts.install_path or error "You must provide the `install_path` option!"
        package.cpath = install_path .. "/udisks/libudisks_xplr.so;" .. package.cpath
    end

    local native = require "udisks.native"

    xplr.fn.custom.udisks = {
        native = native,
        renderContent = function(ctx)
            local kind, body, widths = native.renderContentRows(ctx.layout_size.width, ctx.layout_size.height)
            return {
                [kind] = {
                    ui = { title = { format = "UDisks" } },
                    body = body,
                    widths = widths,
                }
            }
        end,
        up = function()
            native.moveCursor(-1)
        end,
        down = function()
            native.moveCursor(1)
        end,
        changeDirToSelected = function()
            local mount = native.getSelectedMountPointOrComplain()
            if mount then
                return { { ChangeDirectory = mount }, "PopMode" }
            end
        end
    }

    local layout = {
        Horizontal = {
            config = {
                margin = 1,
                constraints = {
                    { Percentage = 70 },
                    { Percentage = 30 },
                }
            },
            splits = {
                { Dynamic = "custom.udisks.renderContent" },
                "HelpMenu",
            },
        },
    }

    local on_key = {
        ["ctrl-r"] = {
            help = "reload",
            messages = {
                { CallLuaSilently = "custom.udisks.native.reload" },
            },
        },
        ["k"] = {
            help = "up",
            messages = {
                { CallLuaSilently = "custom.udisks.up" },
            },
        },
        ["j"] = {
            help = "down",
            messages = {
                { CallLuaSilently = "custom.udisks.down" },
            },
        },
        ["m"] = {
            help = "mount selected",
            messages = {
                { CallLuaSilently = "custom.udisks.native.mountSelected" },
            },
        },
        ["u"] = {
            help = "unmount selected",
            messages = {
                { CallLuaSilently = "custom.udisks.native.unmountSelected" },
            },
        },
        ["g"] = {
            help = "change dir to selected",
            messages = {
                { CallLuaSilently = "custom.udisks.changeDirToSelected" },
            },
        },
    }

    xplr.config.modes.custom.udisks = {
        name = "udisks",
        layout = layout,
        key_bindings = { on_key = on_key },
    }

    xplr.config.modes.builtin.default.key_bindings.on_key[opts.open_key or "D"] = {
        help = "udisks mode",
        messages = {
            { SwitchModeCustom = "udisks" },
        },
    }
end

return M
