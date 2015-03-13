Настройка Web сервера под Windows
=============================================
Под windows проще всего задествовать web сервер IIS, поставляемый вместе с операционной системой.  
Убедитесь, что комоненты web сервера установлены  
{@img install-components.png}  
{@img select-components.png}  

Запустите диспетчер служб IIS  
{@img start-iis-manager.png}  

И в основных настройках дефолтного сайта укажите путь к директории  
**$PROJECT_ROOT\webapps\configurator\client**  
{@img iis-manager.png}  
{@img webapp-path.png}  
Все. Можно открывать в браузере http://localhost

Для целей отладки (если вы планируете работать с клиентским кодом web приложения) рекомендуется указывать именно директорию **$PROJECT_ROOT\webapps\configurator\client**,  а не директорию **$PROJECT_ROOT\webapps\configurator\client\build\production\RtConfigurator**, которая содержит релизный код javascript.  
Это позволит видеть в инструментах разработчика браузера "человеческий" (не обфусцированный) javascript  
{@img debug-js.png}
