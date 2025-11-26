@echo off
chcp 65001 >nul
echo ========================================
echo    PETCONTROL - BUILD SISTEMA INTEGRADO
echo ========================================
echo.

echo Instalando dependencias Node.js...
npm install

echo.
echo Compilando sistema em C...
echo Compilando painel_api.c...
gcc -o painel_api.exe painel_api.c -lraylib -lgdi32 -lwinmm

echo.
echo ========================================
echo    BUILD CONCLUIDO!
echo ========================================
echo.
echo Para executar:
echo   1. node server.js          - Backend Node.js (Porta 5000)
echo   2. painel_api.exe          - Sistema C integrado
echo.
echo Acesse: http://localhost:5000
echo.
pause