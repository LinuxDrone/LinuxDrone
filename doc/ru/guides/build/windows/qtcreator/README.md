Сборка под Windows в QT Creator
=============================================

## Зависимости
Среду разработки QT Creator можно **[получить здесь](https://www.qt.io/download-open-source/)**

О том где взять библиотеки и заголовочные файлы, требуемые для сборки проекта, можно прочитать
**[здесь](#!/guide/buildWindowsCommon-section-%D0%A1%D1%82%D0%BE%D1%80%D0%BE%D0%BD%D0%BD%D0%BD%D0%B8%D0%B5-%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA%D0%B8%2C-%D1%82%D1%80%D0%B5%D0%B1%D1%83%D0%B5%D0%BC%D1%8B%D0%B5-%D0%B4%D0%BB%D1%8F-%D1%81%D0%B1%D0%BE%D1%80%D0%BA%D0%B8-%D0%B8-%D0%B7%D0%B0%D0%BF%D1%83%D1%81%D0%BA%D0%B0-%D0%B8%D1%81%D0%BF%D0%BE%D0%BB%D0%BD%D1%8F%D0%B5%D0%BC%D1%8B%D1%85-%D1%84%D0%B0%D0%B9%D0%BB%D0%BE%D0%B2-%D0%BF%D0%BE%D0%B4-windows)**  

## Сборка
Первым делом, присваиваем переменной **PLATFORM** значение **MSVC** в файле **CMakeLists.txt**, в корне проекта.  
{@img set-platform.png}  

Убедимся, что в списке тулчейнов присутсвует необходимый для нашей целевой платформы.  
{@img toolchains.png}  

Если у вас уже имеется хоть один таргет, то добавим новый  
{@img add-target.png}  
или откроем в QT Creator файл *$PROJECT_ROOT\CMakeLists.txt*
{@img open-project.png}  

Мастер попросит указать каталог сборки  
{@img build-folder.png}
Русские буквы и пробелы недопустимы в пути  
Пример:  
**D:\Projects\build-LinuxDrone-Desktop-AMD64**  

Теперь необходимо передать некоторые параметры для утилиты cmake
{@img cmake.png}
Пример строки с параметрами cmake:  
**-DCMAKE_MAKE_PROGRAM="C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/amd64/nmake.exe" -DBOARD_TYPE=desktop**  

Убедимся, что в QT Creator появился новый таргет
{@img new-target.png}  

Все. Можно жать "Собрать всё" (Ctrl+Shift+B)
