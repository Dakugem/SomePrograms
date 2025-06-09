@echo off
chcp 65001
title Launch File
color 0F
cls

echo Выберите параметры для контроллера:
echo 1) Все включено
echo 2) Без записи CSV файла
echo 3) Без машинки и записи CSV файла
echo 4) Без машинки и записи CSV файла, с подробным выводом с геймпада
echo 5) Только сервер
echo 6) Без сервера и записи CSV файла
echo 7) Без сервера и записи CSV файла, с подробным выводом с геймпада
echo 8) Голосовое управление с распознаванием текста, без записи CSV
echo 9) Голосовое управление с распознаванием цифр, без записи CSV
echo 10) Тест голосового управления с распознаванием текста, только сервер
echo 11) Тест голосового управления с распознаванием цифр, только сервер

set /p config= 

if %config% equ  1 start "Controller" cmd /c Controller.exe
if %config% equ  2 start "Controller" cmd /c Controller.exe no_csv 
if %config% equ  3 start "Controller" cmd /c Controller.exe no_bt no_csv
if %config% equ  4 start "Controller" cmd /c Controller.exe no_bt no_csv detail
if %config% equ  5 start "Controller" cmd /c Controller.exe no_hid no_bt no_csv detail
if %config% equ  6 start "Controller" cmd /c Controller.exe no_serv no_csv 
if %config% equ  7 start "Controller" cmd /c Controller.exe no_serv no_csv detail
if %config% equ  8 start "Controller" cmd /c Controller.exe no_csv voice
if %config% equ  9 start "Controller" cmd /c Controller.exe no_csv voice
if %config% equ 10 start "Controller" cmd /c Controller.exe no_hid no_bt no_csv voice
if %config% equ 11 start "Controller" cmd /c Controller.exe no_hid no_bt no_csv voice

rem if %config% lss 6 start "Client" cmd /c test_client.exe
if %config% lss  6 start "Client" cmd /c py -3.12 client.py
if %config% equ  8 start "Client" cmd /c py -3.12 voice_client_text.py
if %config% equ  9 start "Client" cmd /c py -3.12 voice_client_digits.py
if %config% equ 10 start "Client" cmd /k py -3.12 voice_client_text.py
if %config% equ 11 start "Client" cmd /c py -3.12 voice_client_digits.py
