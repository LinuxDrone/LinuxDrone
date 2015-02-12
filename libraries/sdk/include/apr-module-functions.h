#pragma once

#define RT_HEAP int
#define RT_EVENT int
#define RT_MUTEX int
#define RT_QUEUE int
#define RT_TASK int
#define RT_COND int
#define RTIME int
#define TM_INFINITE int
#define XNOBJECT_NAME_LEN 32



#include <bson.h>
//#include <native/task.h>
//#include <native/queue.h>
//#include <native/heap.h>
//#include <native/event.h>
//#include <native/mutex.h>
//#include <native/cond.h>
#include "common-params.h"

#define RT_HEAP int
#define RT_EVENT int
#define RT_MUTEX int
#define RT_QUEUE int
#define RT_TASK int
#define RT_COND int
#define RTIME int

#define TM_INFINITE 0
#define XNOBJECT_NAME_LEN 32
#define H_PRIO 0
#define H_SHARED 0
#define EV_PRIO 0
#define EV_ALL 0
#define RT_EVENT_INFO int
#define RT_HEAP_INFO int
#define Q_NORMAL 0
#define TM_NONBLOCK 0
#define Q_FIFO 0


#ifdef __cplusplus
extern "C" {
#endif

#define TASK_MODE  0  /* No flags */
#define TASK_STKSZ 0  /* Stack size (use default one) */

// Константы, представляющие собой строки, которые будучи выведенные в консоль, меняют цвет печатываемых далее символов
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Суфикс прибавляемый к имени инстанса модуля, для образования имени входной очереди
#define SUFFIX_QUEUE "_q"

// Суфикс прибавляемый к имени инстанса модуля, для образования имени главного потока модуля
#define SUFFIX_TASK "_mt"

// Суфикс прибавляемый к имени инстанса модуля, для образования имени потока передачи модуля
#define SUFFIX_TR_TASK "_tt"

// Суфикс прибавляемый к имени инстанса модуля, для образования имени мьютекса, служащего для обмена
// данными между основным (главным) потоком модуля и потоком передачи
#define SUFFIX_EXCHANGE_MUTEX "_exm"

// Суфикс прибавляемый к имени инстанса модуля, для образования имени переменной состояния, служащей для обмена
// данными между основным (главным) потоком модуля и потоком передачи
#define SUFFIX_CONDITION "_c"

// Суфикс прибавляемый к имени инстанса модуля (+имя выходной группы портов),
// для образования имени блока разделяемой памяти, созданного в инстансе поставщике
#define SUFFIX_SHMEM "_sm"

// Суфикс прибавляемый к имени инстанса модуля (+имя выходной группы портов),
// для образования имени сервиса уведомлений, созданного в инстансе поставщике
#define SUFFIX_EVENT "_e"

// Суфикс прибавляемый к имени инстанса модуля (+имя выходной группы портов),
// для образования имени мьютекса, созданного и ассоциированного с разделяемой памятью в инстансе поставщике
#define SUFFIX_MUTEX "_m"

// Тип для хранения битовых маск
// В данной реализации 32 бита. (ограничивает кол-во входных портов модуля)
// В случае необходимости может быть переопределен в 64 бита
#define t_mask unsigned int


// Перечисления состояний буферов обмена между рабочим потоком и потоком передачи
typedef enum {
    Empty, Writing, Transferring, Transferred2Queue, Filled
} StatusObj;


// Пречисление возможных типов данных сериализуемых(десериализуемых) в(из) BSON объекты(ов)
typedef enum {
    field_char,
    field_short,
    field_int,
    field_long,
    field_long_long,
    field_float,
    field_double,
    field_const_char,
    field_bool
} TypeFieldObj;

// Тип функции конвертирующей данные структуры в BSON объект
typedef int (*p_obj2bson)(void* obj, bson_t* bson);

// Тип функции конвертирующей данные BSON объекта в структуру
typedef int (*p_bson2obj)(void* p_module, bson_t* bson);

// Тип функции конвертирующей данные параметров командной строки в структуру
typedef int (*p_argv2obj)(void* p_module, int argc, char *argv[]);

// Тип функции выводящей на печать (в консоль) данные структуры
typedef void (*p_print_obj)(void* obj);

// Тип функции выводящей на печать help
typedef void (*p_print_help)();

// Тип функции возвращающей маску входного порта, по его имени
typedef t_mask (*p_get_inputmask_by_inputname)(const char* input_name);

/**
 * @brief
 * \~russian Структура данных, необходимых для записи в блок разделяемой памяти
 */
typedef struct {
    RT_HEAP h_shmem;

    /**
     * @brief
     * \~russian Указатель на блок памяти
     */
    void* shmem;

    /**
     * @brief
     * \~russian Длина блока памяти
     */
    int shmem_len;

    RT_EVENT eflags;

    /**
     * @brief
     * \~russian мьютех используемый для синхронизации при обмене через разделяемую память,
     * потоков разных модулей
     */
    RT_MUTEX mutex_read_shmem;

} shmem_out_set_t;


/**
 * @brief
 * \~russian Структура данных, содержащая информацию о поле в объекте, который ожидает получить подписчик (через очередь).
 * Список таких описаний используется при формировании bson объекта, передаваемого модулю подписчику
 */
typedef struct {
    /**
     * @brief
     * \~russian Тип поля
     */
    TypeFieldObj type_field_obj;

    /**
     * @brief
     * \~russian Имя поля в bson принимаемом через разделяемую память объекте
     */
    char* remote_field_name;

    /**
     * @brief
     * \~russian Смещение данных в структуре локального input объекта
     */
    unsigned short offset_field_obj;

} remote_in_obj_field_t;


/**
 * @brief
 * \~russian Структура данных, необходимых для чтения из блока разделяемой памяти
 */
typedef struct {
    shmem_out_set_t remote_shmem;

    /**
     * @brief
     * \~russian Флаг. Установлена связь (bind) с блоком разделяемой памяти, пренадлежащим инстансу поставщику
     */
    bool f_shmem_connected;

    /**
     * @brief
     * \~russian Флаг. Установлена связь (bind) с сервисом уведомлений, пренадлежащим инстансу поставщику
     */
    bool f_event_connected;

    /**
     * @brief
     * \~russian Флаг. Установлена связь (bind) с мьютексом, пренадлежащим инстансу поставщику
     */
    bool f_mutex_connected;

    /**
     * @brief
     * \~russian Имя инстанса которому принадлежит данная разделяемая память
     */
    char* name_instance;

    /**
     * @brief
     * \~russian Имя группы портов которую представляет данная разделяемая память
     */
    char* name_outgroup;

    /**
     * @brief
     * \~russian Определяет входные порты ассоциированные с данным блоком разделяемой памяти.
     * \~russian То есть для указанных в маске входных портов, данные должны добываться из данного блока разделяемой памяти
     */
    t_mask assigned_input_ports_mask;

    /**
     * @brief
     * \~russian Массив указателей на структуры содержащие информацию о входящих связях с инстансом поставщиком (через разделяемую память)
     */
    remote_in_obj_field_t** remote_in_obj_fields;

    /**
     * @brief
     * \~russian Список полей (в составе bson объекта) которые желает получать через очередь модуль подписчик.
     * \~russian Текущая длина массива remote_in_obj_fields
     */
    size_t len_remote_in_obj_fields;

} shmem_in_set_t;


/**
 * @brief
 * \~russian Структура данных, содержащая информацию о поле в объекте, который ожидает получить подписчик (через очередь).
 * Список таких описаний используется при формировании bson объекта, передаваемого модулю подписчику
 */
typedef struct {
    /**
     * @brief
     * \~russian Смещение данных в структуре объекта источника
     */
    unsigned short offset_field_obj;

    /**
     * @brief
     * \~russian Тип поля (по типу поля определяется длина блока данных представляющих значение данного типа)
     */
    TypeFieldObj type_field_obj;

    /**
     * @brief
     * \~russian Имя поля в bson объекте (которое ожидает подписчик)
     */
    char* remote_field_name;
} remote_out_obj_field_t;


/**
 * @brief
 * \~russian Структура с информацией о удаленной очереди (входной очереди модуля потребителя)
 */
typedef struct {
    /**
     * @brief \~russian Входная очередь модуля подписчика
     */
    RT_QUEUE remote_queue;

    /**
     * @brief \~russian флаг говорящий, что очередь подсоединена
     */
    bool f_queue_connected;

    /**
     * @brief \~russian Имя очереди ксеномай.
     * Хранение ее здесь (хотя ее копия есть в информационной структуре о очереди ксеномай) обсуловлено тем что,
     * пока не выполнен bind, данное имя не пявляется в структуре ксномая. А bind у нас отложен и выполняется в отдельной функции
     * установления связей
     */
    char* name_instance;

} remote_queue_t;


/**
 * @brief \~russian Структура данных, необходимых для работы со списком линков на очередь подписчика
 */
typedef struct {
    /**
     * @brief \~russian Входная очередь модуля подписчика
     */
    remote_queue_t* out_queue;

    /**
     * @brief \~russian Массив указателей на структуры содержащие информацию о исходящих связях с удаленным инстансом (через очередь)
     */
    remote_out_obj_field_t** remote_out_obj_fields;

    /**
     * @brief \~russian Список полей (в составе bson объекта) которые желает получать через очередь модуль подписчик.
     * Текщая длина массива
     */
    size_t len;

} out_queue_set_t;


/**
 * @brief \~russian Структура данных, представляющая тип выходного объекта
 * Для каждого типа выходного объекта, необходимо иметь в памяти такую обслуживающую структуру.
 */
typedef struct {
    void* obj1;
    StatusObj status_obj1;

    void* obj2;
    StatusObj status_obj2;

    p_obj2bson obj2bson;
    p_bson2obj bson2obj;
    p_print_obj print_obj;

    /**
     * @brief \~russian Данные необходимые для публикации объекта в разделяемую память
     */
    shmem_out_set_t shmem_set;

    /**
     * @brief \~russian Массив указателей на структуры содержащие информацию о очередях модулей потребителей
     * Список очередей (подписчиков), в которые будет передаваться объект
     */
    out_queue_set_t** out_queue_sets;

    /**
     * @brief \~russian Текущая длина массива
     */
    size_t out_queue_sets_len;

} out_object_t;


// Тип функции, возаращающей структуру данных, соответствующую выходу, содержащему именованный выходной порт
typedef out_object_t* (*p_get_outobj_by_outpin)(void* p_module,
                                                const char* name_out_pin, unsigned short* size_of_type,
                                                unsigned short* index_port);

// Тип функции, возвращающей смещение члена в структуре входных данных, по имени соответсвующего (члену структуры) входного порта
typedef int (*p_get_offset_in_input_by_inpinname)(void* module,
                                                  const char* name_inpin);

/**
 * @brief \~russian Определение типа бизнес-функции
 */
typedef void (t_cycle_function)(void *cookie);


/**
 * @brief \~russian Определение типа указателя на бизнес-функцию
 */
typedef t_cycle_function* p_cycle_function;


/**
 * @brief \~russian Определение типа функции-обработчика команды
 */
typedef void (t_cmd_function)(int type_command, void* params);


/**
 * @brief \~russian Определение типа указателя на функцию-обработчик команды
 */
typedef t_cmd_function* p_cmd_function;


/**
 * @brief \~russian Структура содержит информацию о всех блоках разделяемой памяти поставщиков
 */
typedef struct {
    /**
     * @brief
     * \~russian Массив указателей на структуры содержащие информацию о разделяемой памяти, контролируемой инстансами поставщиками.
     * Входящие связи (посредством разделяемой памяти).
     * При инициализации данного модуля (обработке списка связей из конфигурации), данный массив
     * заполняется при помощи функции register_remote_shmem
     */
    shmem_in_set_t** remote_shmems;

    /**
     * @brief \~russian Длина массива remote_shmems
     */
    size_t remote_shmems_len;

    /**
     * @brief \~russian Флаг показывающий (если true), что все входящие (через разделяемую память) соединения модуля установлены
     */
    bool f_connected_in_links;

} ar_remote_shmems_t;


/**
 * @brief \~russian Корневая структура, которая содержит всю информацию о инстансе модуля
 */
typedef struct {
    /**
     * @brief \~russian Указатель на подгруженную библиотеку, в которой располагается код моудля.
     * Необходимо хранить для того, чтобы при выгрузке модуля вызвать dlclose и освободить ресурсы
     */
    void *dll_handle;

    /**
     * @brief \~russian Имя типа модуля (например: c-gy87)
     */
    char* module_type;

    /**
     * @brief \~russian Имя инстанса модуля (например: c-gy87-1)
     */
    const char* instance_name;

    /**
     * @brief
     * \~english Main task
     * \~russian Таск бизнес-функции
     */
    RT_TASK task_main;

    /**
     * @brief
     * \~english Transmit task
     * \~russian Таск функции передачи
     */
    RT_TASK task_transmit;

    /**
     * @brief
     * \~english input queue
     * \~russian Входная очередь сообщений модуля
     */
    RT_QUEUE in_queue;

    /**
     * @brief \~russian Массив указателей на структуры содержащие информацию о
     * очередях модулей потребителей Исходящие связи (посредством очередей).
     * При инициализации данного модуля (обработке списка связей из
     * конфигурации), данный массив заполняется при помощи функции
     * add2ar_remote_queues
     */
    remote_queue_t** remote_queues;

    /**
     * @brief \~russian Длина массива remote_queues
     */
    size_t remote_queues_len;

    /**
     * @brief \~russian Массив указателеей на структуры, представляющие разделяемую память модулей поставщиков
     */
    ar_remote_shmems_t ar_remote_shmems;

    /**
     * @brief \~russian Указатель на массив. Список типов объектов, которые пораждает модуль.
     * Массив структур инициализируется в хелпер методе инициализации в автосгенеренном для модуля коде.
     */
    out_object_t** out_objects;

    /**
     * @brief
     * \~russian Указатель на бизнес-функцию (которая должна выполняться в бесконечном цикле)
     */
    p_cycle_function func;

    /**
     * @brief
     * \~russian Указатель на функцию обработки команды (переданной инстансу модуля)
     */
    p_cmd_function cmd_func;

    /**
     * @brief
     * \~russian мьютех используемый для синхронизации при обмене основного потока с потоком передачи
     * \~english mutex used for the exchange of synchronization with the main flow stream transmission
     */
    RT_MUTEX mutex_obj_exchange;

    /**
     * @brief
     * \~russian для синхронизации при обмене основного потока с потоком передачи в разделяемую память
     */
    RT_COND obj_cond;

    /**
     * @brief
     * \~russian Указатель на структуру (входные данные модуля)
     */
    void* input_data;

    /**
     * @brief \~russian Указатель на структуру с настроечными параметрами модуля
     */
    void* specific_params;

    /**
     * @brief \~russian Набор параметров одинаковый для всех модулей
     * (по типу параметров, не по значению. У каждого инстанса свой набор значений)
     */
    common_params_t common_params;

    /**
     * @brief \~russian
     * Функция переноса данных из структуру конфигурационных параметров в bson объект.
     * Указатель на функцию должен подставляться пользовательским (автогенеренным) кодом
     */
    p_obj2bson params2bson;

    /**
     * @brief \~russian
     * Функция переноса данных из bson объекта с конфигурационными параметрами модуля
     * в структуру конфигурационных параметров.
     * Указатель на функцию должен подставляться пользовательским (автогенеренным) кодом.
     */
    p_bson2obj bson2params;

    /**
     * @brief \~russian
     * Функция переноса данных из параметров командной строки модуля
     * в структуру конфигурационных параметров.
     * Указатель на функцию должен подставляться пользовательским (автогенеренным) кодом.
     */
    p_argv2obj argv2params;

    /**
     * @brief \~russian Печать настроечных параметров модуля в консоль
     */
    p_print_obj print_params;

    /**
     * @brief \~russian Функция печати в консоль входного объекта
     */
    p_print_obj print_input;


    /**
     * @brief \~russian Функция в автогенеренной части модуля,
     * выводящая в консоль help по настроечным параметрам модуля
     */
    p_print_help print_help;


    /**
     * @brief \~russian Функция преобразования принятого из очереди bson объекта
     * в структуру представляющую входной объект.
     * Указатель на функцию должен подставляться пользовательским (автогенеренным) кодом
     */
    p_bson2obj input_bson2obj;

    /**
     * @brief \~russian каждый бит числа соответствцет одному из входов модуля.
     * установленный бит подразумевает запрос (пользовательского кода)
     * на обновление соответствующего биту значения во входной структуре данными
     * из разделяемой памяти
     */
    t_mask refresh_input_mask;

    /**
     * @brief \~russian каждый бит числа соответствцет одному из входов модуля.
     * Установленный бит сигнализирует, что в текущей итерации, значение соответствующее биту обновилось
     */
    t_mask updated_input_properties;

    /**
     * @brief \~russian Функция в автогенеренной части модуля,
     * возвращающая метаданные выходного объекта, по имени выходного порта модуля
     */
    p_get_outobj_by_outpin get_outobj_by_outpin;

    /**
     * @brief \~russian Функция в автогенеренной части модуля,
     * возвращающая смещение поля в структуре входного объекта модуля
     */
    p_get_offset_in_input_by_inpinname get_offset_in_input_by_inpinname;

    /**
     * @brief \~russian Флаг показывающий (если true), что все исходящие соединения модуля установлены
     */
    bool f_connected_out_links;

    /**
     * @brief \~russian функция (реализованная в автосгенеренном коде модуля)
     * возвращает битовую маску, представляющую указанный (в качестве строкового параметра) по имени входной порт
     */
    p_get_inputmask_by_inputname get_inmask_by_inputname;

    /**
     * @brief \~russian функция Время последней попытки установления связей
     */
    RTIME time_attempt_link_modules;

    /**
     * @brief \~russian Строка в формате JSON содержащая определение модуля
     */
    const char* json_module_definition;

} module_t;


/**
 * @brief \~russian Тип функции создания модуля
 */
typedef module_t* (*create_f)(void *);

// Тип функции инициализации модуля
typedef int (*init_f)(module_t*, int argc, char *argv[]);

// Тип функции старта модуля
typedef int (*start_f)(module_t*);

// Тип функции удаления модуля
typedef void (*delete_f)(module_t*);

#ifdef WIN32
__declspec(dllexport) int init(module_t* module, int argc, char *argv[]);
__declspec(dllexport) int init_object_set(shmem_out_set_t * shmem, char* instance_name, char* out_name);
__declspec(dllexport) int start(void* module);
__declspec(dllexport) int stop(void* module);
__declspec(dllexport) void get_input_data(module_t *module);
#else
int init(module_t* module, int argc, char *argv[]);
int init_object_set(shmem_out_set_t * shmem, char* instance_name, char* out_name);
int start(void* module);
int stop(void* module);
void get_input_data(module_t *module);
#endif


int refresh_input(void* p_module);

char *replace(const char *s, char ch, const char *repl);

/**
 * @brief connect_in_links Устанавливает входящие соединения посредством разделяемой памяти
 * с инстансами поставщиками данных
 * @param p_module
 * @return
 */
int connect_in_links(ar_remote_shmems_t* ar_remote_shmems,
                     const char* instance_name);

shmem_in_set_t* register_remote_shmem(ar_remote_shmems_t* ar_remote_shmems,
                                      const char* name_remote_instance, const char* name_remote_outgroup);

int unregister_remote_shmem(ar_remote_shmems_t* ar_remote_shmems,
                            const char* name_remote_instance, const char* name_remote_outgroup);

void read_shmem(shmem_in_set_t* remote_shmem, void* data,
                unsigned short* datalen);

void print_task_receive_error(int err);
void print_task_reply_error(int err);
void print_task_bind_error(int err);
void print_task_send_error(int err);
void print_rt_mutex_bind_error(int err);
void print_rt_mutex_acquire(int err);

void debug_print_bson(char* where, bson_t* bson);

#ifdef __cplusplus
}
#endif
