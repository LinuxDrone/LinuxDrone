Обновление версии ExtJS
=============================================

1. Скачиваем последнюю GPL версию ExtJS.  
Для этого переходим по ссылке [http://www.sencha.com/products/extjs/details](http://www.sencha.com/products/extjs/details) и в самом низу страницы находим секцию **OPEN SOURCE GPL LICENSING** и кнопку **DOWNLOAD**.  
Так как sencha любит перепрятывать ссылку на GPL версию, можно поискать еще здесь
[https://www.sencha.com/legal/GPL](https://www.sencha.com/legal/GPL)  
{@img ExtJS-GPL.png}
Распаковываем zip архив (например в директорию $PROJECT_ROOT\tools)  

2. На той же странице находим, скачиваем и устанавливаем утилиту **Sencha Cmd**.  
{@img Download-Sencha-CMD.png}  
*Если ранее у вас была установлена старая версия Sencha Cmd, убедитесь, что при вызове утилиты в консоли (команда **sencha**), запускается именно последняя версия (т.е. путь прописанный в переменную окружения PATH ведет именно к последней версии Sencha Cmd)*

3. В консоли, переходим в директорию **$PROJECT_ROOT\webapps\configurator\client** и выполняем команду  
**sencha app upgrade $PATH_EXTJS_GPL**  
где $PATH_EXTJS_GPL - путь к директории в которую мы распаковали исходники  ExtJS.  
Например:  
`sencha app upgrade $PROJECT_ROOT/tools/ext-5.0.1`  
Пример удачного обновления на скриншоте
{@img successful-update.png}

4. Ну и на последок собираем проект web приложения, дабы убедиться, что обновление не поломало процесс сборки.  
Пример удачной сборки проекта web приложения  
{@img successful-build.png}
