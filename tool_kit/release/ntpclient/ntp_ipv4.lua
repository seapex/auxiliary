#!/usr/local/bin/lua

local file_path = "/etc/ntp.conf"

--[[
handle the IPv4 address in the ntp.conf
Input： rw - read or write
        new_ipv4 - the new IPv4 address to replace the old one
--]]
function handle_ipv4_address(rw, new_ipv4)
    local file = io.open(file_path, "r")
    if not file then
        print("Could not open file: " .. file_path)
        return
    end
    local content = file:read("*all")
    file:close()

    -- Pattern to match IPv4 addresses
    local ipv4_pattern = "(%d+%.%d+%.%d+%.%d+)"
    
    if rw == "read" then
        return content:match(ipv4_pattern)
    elseif rw == "write" then
        local new_content = content:gsub("server " .. ipv4_pattern, "server " .. new_ipv4)
        file = io.open(file_path, "w")
        if not file then
            print("Could not open file for writing: " .. file_path)
            return
        end
        file:write(new_content)
        file:close()
    else
        print("Invalid operation: " .. rw)
    end    
end
