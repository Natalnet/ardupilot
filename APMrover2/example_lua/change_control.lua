-- This script is a test for AP_Mission bindings
-- run 3 loops of mission; change CBMv; run 3 more loops. end

-- RULES TO CREATE THE MISSION:
  -- first mission point is the first waypoint (no takeoff)
  -- last mission point should be return to home.

local controller_count = 1
local jumps_count = 1

local max_controller_count = 2
local max_jumps_count = 2

-- PARAMETERS OF FIRST CBMv

-- PROPULSION CONTROL
if not(param:set_by_name('SAIL_PROP_CTRL',2)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end


if not(param:set_by_name('SAIL_POLAR_T',1)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- TACK CONTROL

if not(param:set_by_name('SAIL_TACK_TYPE',1)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end


if not(param:set_by_name('SAIL_TACK_DT',20)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end


if not(param:set_by_name('SAIL_TACK_THETAT',60)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end


-- WAYPOINT FOLLOWING

if not(param:set_by_name('SAIL_WAYP_TYPE',0)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- STEERING PID

--if not(param:set_by_name('ATC_STR_RAT_P',1)) then
--  gcs:send_text(0, 'LUA: param set failed')
--else
--end



function update() -- this is the loop which periodically runs

  local mission_state = mission:state()

   -- make sure the mission is running
  if mission_state == mission.MISSION_STOPPED then
    gcs:send_text(0, "LUA: Mission stopped")
    return update, 1000 -- reschedules the loop
  end

  local mission_index = mission:get_current_nav_index()

  -- gcs:send_text(0, string.format("Controller: %d",controller_count))
  -- gcs:send_text(0, string.format("Loop: %d",jumps_count))
  --  gcs:send_text(0, string.format("Mission Index: %d",mission:get_current_nav_index()))

  -- num commands includes home so - 1
  local mission_length = mission:num_commands() - 1

  -- while there are controller to evaluate
  if controller_count <= max_controller_count then

       -- if is going back to home
      if mission_index == mission_length then

        -- if reached maximum number of jumps for that controller
        if jumps_count == max_jumps_count then

        -- SET PARAMETERS OF FIRST CBMv

        -- PROPULSION CONTROL

        if not(param:set_by_name('SAIL_PROP_CTRL',1)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end


        if not(param:set_by_name('SAIL_FIXED_ANGLE',20)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end

        -- TACK CONTROL

        if not(param:set_by_name('SAIL_TACK_TYPE',0)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end


        if not(param:set_by_name('SAIL_XTRACK_MAX',20)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end


        if not(param:set_by_name('SAIL_NO_GO_ANGLE',30)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end


        -- WAYPOINT FOLLOWING

        if not(param:set_by_name('SAIL_WAYP_TYPE',1)) then
          gcs:send_text(0, 'LUA: param set failed')
        else
        end

        -- STEERING PID

        --if not(param:set_by_name('ATC_STR_RAT_P',1)) then
        --  gcs:send_text(0, 'LUA: param set failed')
        --else
        --end

          -- increment controller count
          controller_count = controller_count + 1

          -- resets jumps count
          jumps_count = 1

          -- jumps to first waypoint
          local set_succeed = mission:set_current_cmd(1)

        else
          -- jumps to first waypoint
          local set_succeed = mission:set_current_cmd(1)

          -- increment jumps count
          jumps_count = jumps_count + 1
        end
      end
  -- if finished evaluating all controllers
  else

    -- disable scripting
    gcs:send_text(0, 'LUA: goodby, world')
    param:set_by_name('SCR_ENABLE',0)

    -- go home
    mission:set_current_cmd(mission:num_commands() - 1)

  end

 return update, 1000 -- reschedules the loop
end

return update() -- run immediately before starting to reschedule