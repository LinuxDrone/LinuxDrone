Предварительные требования при сборке системы на Windows машине
=============================================

## Стороннние библиотеки, требуемые для сборки и запуска исполняемых файлов под Windows

Для сборки модулей и сервисов, запускаемых под Windows, требуются некоторые сторонние библиотеки:

[https://apr.apache.org/](https://apr.apache.org/)  
[https://github.com/mongodb/libbson](https://github.com/mongodb/libbson)  
[https://libwebsockets.org/trac/libwebsockets](https://libwebsockets.org/trac/libwebsockets)  

Дабы не собирать данные библиотеки самостоятельно, вы можете скачать скомпилированные версии данных библиотек (и заголовочных файлов) в виде zip архива.  

**[Скачать](http://rt-platform.org/rootfs/rootfs-desktop-win.zip)**

Распаковываем скачанный архив *rootfs-desktop-win.zip* в директорию $PROJECT_ROOT\tools  
У вас должна получиться следующая иерархия директорий в  tools
{@img folders-hierarchy.png}  

Этого достаточно для сборки, но вам понадобится запускать собранные модули, которым требуются для работы скачанные DLL. Поэтому следует добавить в переменную окружения PATH, пути к следующим директориям.

**$PROJECT_ROOT\tools\rootfs\desktop\libwebsockets\bin\win\x64**  
**$PROJECT_ROOT\tools\rootfs\desktop\libapr\bin\win\x64**  
**$PROJECT_ROOT\tools\rootfs\desktop\libbson\bin\win\x64**  

Пример:  
*D:\Projects\testBuild\LinuxDrone\tools\rootfs\desktop\libwebsockets\bin\win\x64;D:\Projects\testBuild\LinuxDrone\tools\rootfs\desktop\libapr\bin\win\x64;D:\Projects\testBuild\LinuxDrone\tools\rootfs\desktop\libbson\bin\win\x64*  
