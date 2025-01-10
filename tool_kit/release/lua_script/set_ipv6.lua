#!/usr/local/bin/lua

local idx = 1

-- Input:  flag, [ipv6_address], [gateway], [netmask]
local function set_ipv6(...)
    local args = {}
    for i, v in ipairs({...}) do args[i] = v end
    local flag = args[idx]
    idx = idx + 1
    
    local file_path = "/etc/network/interfaces"
    -- local file_path = "./interfaces"
    local lines = {}
    
    -- Read the current content of the file
    for line in io.lines(file_path) do
        table.insert(lines, line)
    end

    -- Modify the lines based on the flag
    local found_iface = false
    local cnt = 1
    for i, line in ipairs(lines) do
        if line:match("^iface eth0 inet6 static") then
            found_iface = true
        elseif found_iface then
            if line:match("^%s*address%s+") and bit32.band(flag, 1) ~= 0 then
                lines[i] = "    address " .. args[idx]
                idx = idx + 1
            elseif line:match("^%s*gateway%s+") and bit32.band(flag, 2) ~= 0 then
                lines[i] = "    gateway " .. args[idx]
                idx = idx + 1
            elseif line:match("^%s*netmask%s+") and bit32.band(flag, 4) ~= 0 then
                lines[i] = "    netmask " .. args[idx]
                idx = idx + 1
            end
            cnt = cnt + 1
            -- Stop modifying after the relevant section
            if cnt > 3 or line:match("^%s*iface%s+") then
                break
            end
        end
    end
    -- If the interface section was not found, add it
    if not found_iface then
        for i, line in ipairs(lines) do
            if line:match("^%s*mtu%s+1400") then
                table.insert(lines, i + 1, "iface eth0 inet6 static")
                table.insert(lines, i + 2, "    address " .. (bit32.band(flag, 1) ~= 0 and args[idx] or "fd6d:ff70:f643:b0e0::ec"))
                if bit32.band(flag, 1) ~= 0 then idx = idx + 1 end
                table.insert(lines, i + 3, "    gateway " .. (bit32.band(flag, 2) ~= 0 and args[idx] or "fd6d:ff70:f643:b0e0::1"))
                if bit32.band(flag, 2) ~= 0 then idx = idx + 1 end
                table.insert(lines, i + 4, "    netmask " .. (bit32.band(flag, 4) ~= 0 and args[idx] or "64"))
                if bit32.band(flag, 4) ~= 0 then idx = idx + 1 end
                break
            end
        end
    end
    -- Write the modified content back to the file
    local file = io.open(file_path, "w")
    for _, line in ipairs(lines) do
        file:write(line .. "\n")
    end
    file:close() --]]
end

function set_ntp_server(new_ipv6)
    local file_path = "/etc/ntp.conf"
    local file = io.open(file_path, "r")
    if not file then
        print("Could not open file: " .. file_path)
        return
    end
    local content = file:read("*all")
    file:close()

    -- Pattern to match IPv6 addresses
    local ipv6_pattern = "(%x+:[%x:]+)"
    
    local new_content = content:gsub("server " .. ipv6_pattern, "server " .. new_ipv6)
    file = io.open(file_path, "w")
    if not file then
        print("Could not open file for writing: " .. file_path)
        return
    end
    file:write(new_content)
    file:close()
end


-- arg[1] = flag, bit0: address, bit1: gateway, bit2: netmask, bit3: ntp server
set_ipv6(arg[1], arg[2], arg[3], arg[4])
if bit32.band(arg[1], 8) ~= 0 then
    set_ntp_server(arg[idx])
end

-- Example usage:
-- ./set_ipv6.lua 1 fd6d:ff70:f643:1::ea
-- ./set_ipv6.lua 2 fd6d:ff70:f643:1::1
-- ./set_ipv6.lua 4 64
-- ./set_ipv6.lua 8 fd6d:ff80::1
-- ./set_ipv6.lua 3 "2001:db8::3" "2001:db8::1"
-- ./set_ipv6.lua 5 "2001:db8::3" "64"
-- ./set_ipv6.lua 6 "2001:db8::1" "64"
-- ./set_ipv6.lua 7 "fd6d:ff70:f643:1::ea" "fd6d:ff70:f643:1::1" "64"

-- ./set_ipv6.lua 1 fd6d:ff70:f643:1::ea
-- ./set_ipv6.lua 15 "fd6d:ff70:f643:1::ea" "64" "fd6d:ff70:f643:1::1"
