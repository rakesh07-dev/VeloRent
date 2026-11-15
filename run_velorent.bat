@echo off
cd /d "%~dp0server"
start "" http://localhost:3001
node index.js
