@echo off
color 9f
title Survival Server by Dark-bart
echo :: If Survival Server shutdown, the Survival Server will restart.
echo :: Auto-Restarter did by Dark-bart.
:begin
SurvivalServer.exe
echo  -- Survival Server shutdown, restarting... --
goto begin
:goto begin