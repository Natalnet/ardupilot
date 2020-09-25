-- teste busca extrema

-- ### CONTROLE DE PROPULSÃO

-- SAIL_PROP_CTRL
--        LINEAR  = 0,
--        FIXED = 1,
--        IMPROVED_EXTREMUM_SEEKING = 2,
--        POLAR_DIAGRAM_REAL = 3,
--        EXTREMUM_SEEKING = 4
if not(param:set_by_name('SAIL_PROP_CTRL',2)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- ## LINEAR

-- ## FIXED
if not(param:set_by_name('SAIL_FIXED_ANGLE',30)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- ## IMPROVED_EXTREMUM_SEEKING

-- intervalo = 1 s
if not(param:set_by_name('SAIL_POLAR_T',1)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

--## EXTREMUM_SEEKING

-- passo do angulo da vela
if not(param:set_by_name('SAIL_EXTR_STEP',5)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- intervalo = 1 s
if not(param:set_by_name('SAIL_EXTR_T',1)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end


-- ### CONTROLE DE ORIENTACAO


-- ### SEGUIMENTO DE WAYPOINT

-- SAIL_WAYP_TYPE
--        L_ONE  = 0,  (SEGUIMENTO DE LINHA)
--        HEADING = 1  (orientacoes sucessivas)
if not(param:set_by_name('SAIL_WAYP_TYPE',0)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- ## SEGUIMENTO DE LINHA

-- ## ORIENTACAO sucessivas


-- ### BORDEJAR

-- SAIL_TACK_TYPE
--        REACTIVE  = 0,
--        DELIBERATIVE = 1
if not(param:set_by_name('SAIL_TACK_TYPE',0)) then
  gcs:send_text(0, 'LUA: param set failed')
else
end

-- ## REACTIVE

-- distance lateral do bordejo reativo
if not(param:set_by_name('SAIL_XTRACK_MAX',20)) then
  gcs:send_text(0, 'LUA: param set failed SAIL_XTRACK_MAX')
else
end

-- angulo mais perto da linha do vento que o veleiro consegue navegar (limite da zona morta)
if not(param:set_by_name('SAIL_NO_GO_ANGLE',30)) then
  gcs:send_text(0, 'LUA: param set failed SAIL_NO_GO_ANGLE')
else
end

-- angulo adicioal para o bordejar (total = sail_no_go_angle + sail_nogo_pad)
if not(param:set_by_name('SAIL_NOGO_PAD',30)) then
  gcs:send_text(0, 'LUA: param set failed SAIL_NOGO_PAD')
else
end

-- ## DELIBERATIVE

-- distance lateral do bordejo deliberativo
--if not(param:set_by_name('SAIL_TACK_DT',20)) then
--  gcs:send_text(0, 'LUA: param set failed')
--else
--end

-- angulo durante o bordejo deliberativo
--if not(param:set_by_name('SAIL_TACK_THETAT',60)) then
--  gcs:send_text(0, 'LUA: param set failed')
--else
--end