set LUPDATEPATH=C:\Qt\5.6.3\5.6.3\mingw49_32\bin\
set SOURCE_DIR=../../Src
set GUI_DIR=../../GUI

%LUPDATEPATH%/lupdate %SOURCE_DIR% %GUI_DIR% -extensions cpp,h,ui -ts trans_de-DE.ts
pause
