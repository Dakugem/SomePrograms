# Эмулятор процессора BytePusher
**BytePusher** имеет только одну команду - **BBJ** (Byte Byte Jump).  
Аргументы команды **BBJ** имеют одинаковый размер в байтах, конкретно в данном случае по **3 байта** каждый.  
- Первый аргумент - с какого адреса считать данные (какой байт прочитать из памяти).  
- Второй аргумент - куда записать прочитанные данные.  
- Третий аргумент - с какого адреса начинается следующая команда **BBJ**.

В программе сделано считывание программы для эмулятора из файла `memory_init.hex`, выгрузка результата работы в файл `result.hex`, но, на данный момент, нет графического и какого-либо еще отображения этой самой работы.  
