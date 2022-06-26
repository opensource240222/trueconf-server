
--require "constants" -- this string is only to simplify script development, comment it out when done.

function make_terminal_storage()
  local t = {}
  t.by_pattern = {}
  t.by_position = {} -- every element of this table contains 'pattern' and 'func' field
  return t
end

-- table for sip terminals data
sip_table = make_terminal_storage()

-- table for H323 terminals data
h323_table = make_terminal_storage()

-- table for RTSP devices
rtsp_table = make_terminal_storage()

--[[
This function accepts table of codecs to remove as first parameter
and all available codecs string as second.

Returns result string with all allowed codecs.
]]
function remove_codecs(to_remove, available_codecs)
  local split = function(str)
    local t = {}
    local n = 1
    for w in string.gmatch(str, "%S+") do
      t[n] = w
      n = n + 1
    end
    return t
  end
  local codecs = split(available_codecs)

  local is_removable_codec = function(c)
    for _, r in ipairs(to_remove) do
      if c == r then
        return true
      end
    end
    return false
  end

  local res = ""
  for _, c in ipairs(codecs) do
    if is_removable_codec(c) == false then
      res = res .. c .. " "
    end
  end

  return res
end

--[[ Protocol specific functions ]]

--[[
Every corrector function accepts Terminal Identifier (User Agent, Vendor String) as argument
and returns CallConfig data
]]

function make_adder(storage)
  local s = storage
  return function(pattern, func)
    local t = s.by_pattern[pattern] 
    if not t  then -- create new value
      t = {}
      t.pattern = pattern
      t.func = func
      s.by_pattern[pattern] = t
      s.by_position[(#s.by_position + 1)] = t
    else -- update old one
      t.pattern = pattern
      t.func = func
    end
  end
end

-- handy adder functions
add_H323_device = make_adder(h323_table)
add_SIP_device  = make_adder(sip_table)
add_RTSP_device = make_adder(rtsp_table)


function get_device_config(config_storage, terminal_id)
  -- exact match only
  local data = config_storage.by_pattern[terminal_id]
  if data and data.func then
    return data.func(terminal_id)
  end

  -- try to apply patterns one by one until success
  for _, v in ipairs(config_storage.by_position) do
    if string.find(terminal_id, v.pattern) then
      return v.func(terminal_id)
    end
  end

  return nil
end

--[[ Entry Point ]]
function get_call_config_data(protocol_id, terminal_id)
  local cfg = nil -- call config data
  if protocol_id == SIGNAL_PROTOCOL_SIP then
    -- handle SIP terminal
    return get_device_config(sip_table, terminal_id)
  elseif protocol_id == SIGNAL_PROTOCOL_H323 then
    -- handle h323 terminal
    return get_device_config(h323_table, terminal_id)
  elseif protocol_id == SIGNAL_PROTOCOL_RTSP then
    -- handle RTSP device
    return get_device_config(rtsp_table, terminal_id)
  end

  -- return call config data, if any
  return cfg
end

--[[ End of common code ]]
--[[======================================================================]]

--[[ !!! terminal definitions !!! ]]

--[[ === SIP devices === ]]

-- [[ Polycom ]]
-- Polycom VVX-1500
-- CIF resolution issues found at versions:
-- PolycomVVX-VVX_1500-UA/3.2.2.0481
-- PolycomVVX-VVX_1500-UA/3.3.3.0072
add_SIP_device("PolycomVVX%-VVX_1500%-UA/.+",
               function(user_agent)
                  local cfg = {}
                  cfg["H264 To Terminal Video Width"] = 352
                  cfg["H264 To Terminal Video Height"] = 288
                  return cfg
                end
)

--[[ Grandstream ]]

-- Grandstream GVC3200
add_SIP_device("Grandstream GVC3200.+",
               function (user_agent)
                 local cfg = {}
                 cfg["BFCP Protocol"] = CONNECTIONTYPE_UDP
                 return cfg
               end
)

--[[ Lifesize ]]
-- SIP: LifeSize Express 220
-- Example: LifeSize Express_220/LS_EX2_4.7.11 (4)
add_SIP_device("LifeSize Express_220/.+",
               function (user_agent)
                 local cfg = {}
                 cfg["Enabled Codecs"] = remove_codecs({"G729a"}, DEFAULT_ENABLED_CODECS)
                 return cfg
               end
)
--[[ huawei ]]
 -- SIP: Huawei-MC850/V100R001C02B120
add_SIP_device("Huawei%-MC850/.+",
               function (user_agent)
                 local cfg = {}
                 cfg["Enabled Codecs"] = remove_codecs({"SIREN14_32","SIREN14_48","SIREN14_24"}, DEFAULT_ENABLED_CODECS)
                 return cfg
               end
)
--[[ Panasonic ]]
-- SIP: KX-VC1300_HDVC-MPCS

add_SIP_device("KX%-VC1300_HDVC%-MPC.+",
               function (user_agent)
                 local cfg = {}
                 cfg["Enabled Codecs"] = remove_codecs({"XH264UC"}, DEFAULT_ENABLED_CODECS)
                 cfg["BFCP Protocol"] = CONNECTIONTYPE_UDP
                 return cfg
               end
)

--[[ Ericsson ]]

-- Example: "Ericsson-LG Enterprise iPECS-eMG eMG800 3.1.10"
add_SIP_device("Ericsson%-LG Enterprise iPECS.+",
               function (user_agent)
                 local cfg = {}
                 cfg["H264 Payload Type"] = 98
                 return cfg
               end
)

--[[ Soft-phones ]]

-- Linphone is known to misbehave with enabled BFCP.
-- Example UA: "Linphone/3.9.1 (belle-sip/1.4.2)"
add_SIP_device("Linphone/.+",
               function (user_agent)
                 local cfg = {}
                 cfg["BFCP Enabled"] = false
                 return cfg
               end
)

--[[ Yealink VCDesktop ]]
add_SIP_device("Yealink VCDesktop 1.+",
               function (user_agent)
                 local cfg = {}
                 cfg["Siren Swap Bytes"] = true
                 return cfg
               end
)

--[[ Lync/Skype for business ]]

add_SIP_device("UCCAPI/.+",
               function (user_agent)
                 local cfg = {}
                 cfg["ICE Enabled"] = true
                 cfg["SRTP Enabled"] = true
                 cfg["BFCP Enabled"] = false
                 cfg["H224 Enabled"] = false
                 cfg["Protocol"] = "TLS"
                 return cfg
               end
)

add_SIP_device("RTC/.+",
               function (user_agent)
                 local cfg = {}
                 cfg["ICE Enabled"] = true
                 cfg["SRTP Enabled"] = true
                 cfg["BFCP Enabled"] = false
                 cfg["H224 Enabled"] = false
                 cfg["Protocol"] = "TLS"
                 return cfg
               end
)


--[[ === H323 devices === ]]

--[[ LifeSize ]]
-- H323: LifeSize Express 220
-- Example: LifeSize Express 220::4.7.11.4
add_H323_device("LifeSize Express 220::.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)

-- H323: LifeSize Room 220
-- Example: LifeSize Room 220::4.10.0.49
add_H323_device("LifeSize Room 220::.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)

-- H323: LifeSize Team 220
-- Example: LifeSize Team 220::4.12.3.4"
-- Example: LifeSize Team 220::4.9.0.73
add_H323_device("LifeSize Team 220::.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)

-- Example: LifeSize Express 220::4.11.13.1"
add_H323_device("LifeSize Express 220::.",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)

add_H323_device("LifeSize Bridge 2200::.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)

add_H323_device("LifeSize Passport 2::.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["Siren Swap Bytes"] = true
                  return cfg
                end
)


--[[ Sony ]]
-- CIF resolution issues found at versions:
-- terminal Sony PCS-TL30: "SONY PCS-101 00::PCS-G70 2009-11-30 17:00 Ver 02.70      \r\n"
-- terminal Sony PCS-G50:  "SONY PCS-101 00::PCS-G70 2005-06-10 09:20 Ver 02.10      \r\n"
add_H323_device("SONY PCS%-101 00::PCS%-G70.+",
                function (product_and_version)
                  local cfg = {}
                  cfg["H264 To Terminal Video Width"] = 352
                  cfg["H264 To Terminal Video Height"] = 288
                  return cfg
                end
)

-- [[ Polycom ]]
-- Polycom VVX-1500. Example pID+vID: VVX 1500::3.3.3.0072
add_H323_device("VVX 1500::.+",
               function(user_agent)
                  local cfg = {}
                  cfg["H264 To Terminal Video Width"] = 352
                  cfg["H264 To Terminal Video Height"] = 288
                  return cfg
                end
)

--[[ === RTSP devices === ]]


--[[ === TESTING ===  ]]

-- Polycom PVX 8.X
--[[
add_H323_device("Polycom ViaVideo::Release 8%..+",
                function(product_and_version)
                  local t = {}
                  return t
                end
)

add_SIP_device("Polycom VV 8%..+",
               function(user_agent)
                 local t = {}
                 return t
               end
)
]]

