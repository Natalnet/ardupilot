-- This script is a test of param set and get
local count = 0

 function update() -- this is the loop which periodically runs

   -- get and print all the scripting paramerters
  --local value = param:get_by_name('SAIL_PROP_CTRL')
  --if value then
  --  gcs:send_text(0, string.format('LUA: SAIL_PROP_CTRL: %i',value))
  --else
  --  gcs:send_text(0, 'LUA: get SAIL_PROP_CTRL failed')
  --end

   -- increment the script heap size by one
  local prop_control = param:get_by_name('SAIL_PROP_CTRL')

  if prop_control then
      -- loop in 4 values
    if prop_control > 3 then
      prop_control = 0;
    else
      prop_control = prop_control + 1
    end

    if not(param:set_by_name('SAIL_PROP_CTRL',prop_control)) then
      gcs:send_text(0, 'LUA: param set failed')
    else
      gcs:send_text(0, string.format('LUA: SAIL_PROP_CTRL: %i',prop_control))
    end

  else
    gcs:send_text(0, 'LUA: get SAIL_PROP_CTRL failed')
  end

   return update, 30*1000 -- reschedules the loop
end

 return update() -- run immediately before starting to reschedule