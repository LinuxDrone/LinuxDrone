# Генерация исходного кода вспомогательных функций модуля

Файлы содержащие исходный код helper функций модуля, генерируются при помощи скрипта
**LinuxDrone\libraries\sdk\ModuleGenerator.js**

Результатом работы скрипта являются два файла:

- **YOUR_MODULE.helper.h** Заголовочный файл
- **YOUR_MODULE.helper.c** Файл с исходным кодом

`Функция main() размещается в YOUR_MODULE.helper.c`

## Параметры скрипта

Пример вызова:
`node ModuleGenerator.js YOUR_MODULE.def.json OUT_DIR PLATFORM`

- **YOUR_MODULE.def.json** Файл определения модуля
- **OUT_DIR** Директория в которую будут записаны сгенерированные файлы
- **PLATFORM** Принимает значения - XENO, MSVC, GCC  
XENO - Если необходимо сгенерить исходники использующие библиотеку Xenomai (Только для Linux)  
GCC - Для генерации исходников использующих библиотеку APR. Компилятор GCC (Linux, Mac OS X)  
MSVC - Для генерации исходников использующих библиотеку APR. Компилятор MSVC (Windows)
