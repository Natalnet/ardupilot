function update () -- periodic function that will be called
  current_pos = ahrs:get_position()
  gcs:send_text(0, string.format('SHOW!'))
  home = ahrs:get_home()
  if current_pos and home then
    distance = current_pos:get_distance(ahrs:get_home()) -- calculate the distance from home
    if distance > 1000 then -- if more then 1000 meters away
      distance = 1000;      -- clamp the distance to 1000 meters
    end
    servo.set_output_pwm(96, 1000 + distance) -- set the servo assigned function 96 (scripting3) to a proportional value
  end

  return update, 1000 -- request to be rerun again 1000 milliseconds (1 second) from now
end

return update, 1000 -- request to be rerun again 1000 milliseconds (1 second) from now