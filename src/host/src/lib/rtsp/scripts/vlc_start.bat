@echo on
start C:\"Program Files (x86)"\VideoLAN\VLC\vlc.exe %1
@echo off
C:\Windows\System32\PING.EXE 127.0.0.1 -n 2 > nul
