@echo off
setlocal

rem Prompt for network interface name
set /p InterfaceName="Enter the name of your network interface (e.g., Wi-Fi, Ethernet): "

rem Prompt for primary DNS server
set /p PrimaryDNS="Enter the primary DNS server (e.g., 8.8.8.8): "

rem Prompt for secondary DNS server
set /p SecondaryDNS="Enter the secondary DNS server (e.g., 8.8.4.4): "

echo Changing DNS settings for %InterfaceName%...
netsh interface ip set dns name="%InterfaceName%" source=static addr=%PrimaryDNS%
netsh interface ip add dns name="%InterfaceName%" addr=%SecondaryDNS% index=2

echo DNS settings updated.
echo Primary DNS: %PrimaryDNS%
echo Secondary DNS: %SecondaryDNS%
endlocal
pause