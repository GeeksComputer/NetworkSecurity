@echo off
set /p host=Enter the target host: 
set /p count=Enter the number of pings: 

for /l %%i in (1,1,%count%) do (
	ping -n 1 %host%
)