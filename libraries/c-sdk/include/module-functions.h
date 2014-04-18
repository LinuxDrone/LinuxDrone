#pragma once

#include <native/task.h>
#include <bson.h>
#include <native/queue.h>
#include <native/heap.h>
#include <native/event.h>
#include <native/mutex.h>
#include <native/cond.h>

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

typedef enum
{
    Empty,
    Writing,
    Transferring,
    Filled
} StatusObj;

typedef enum
{
    fieldInteger,
    fieldBoolean
} TypeFieldObj;


typedef int (*p_obj2bson)(void* obj, bson_t* bson);

// примечание: буфер объекта добывается из структуры модуля
typedef int (*p_bson2obj)(void* p_module, bson_t* bson);

typedef void (*p_print_obj)(void* obj);

/**
  Структура данных, необходимых для работы с блоком разделяемой памяти
 */
typedef struct
{
    RT_HEAP h_shmem;

    void* shmem;
    int shmem_len;

    RT_EVENT eflags;

    /**
     * @brief mutex_read_shmem
     * /~russian мьютех используемый для синхронизации при обмене через разделяемую память,
     * потоков разных модулей
     */
    RT_MUTEX mutex_read_shmem;

} shmem_set_t;


/**
  Структура данных, содержащая информацию о поле в объекте, который ожидает получить подписчик (через очередь)
  Список таких описаний используется при формировании bson объекта, передаваемого модулю подписчику
 */
typedef struct
{
    unsigned short  offset_field_obj;    // Смещение данных в структуре объекта источника
    TypeFieldObj    type_field_obj;      // Тип поля (по типу поля определяется длина блока данных представляющих значение данного типа)
    char*           remote_field_name;   // Имя поля в bson объекте (которое ожидает подписчик)

} remote_obj_field_t;
typedef struct
{
    // Текщая длина массива
    size_t len;

    // Массив указателей на структуры содержащие информацию о очередях модулей потребителей
    remote_obj_field_t** remote_obj_fields;

} sized_ar_remote_obj_field_t;



/**
 * Структура с информацией о удаленной очереди (входной очереди модуля потребителя)
 */
typedef struct
{
    // Входная очередь модуля подписчика
    RT_QUEUE remote_queue;

    // флаг говорящий, что очередь подсоединена
    bool f_queue_connected;

    // Имя очереди ксеномай.
    // Хранение ее здесь (хотя ее копия есть в информационной структуре о очереди ксеномай) обсуловлено тем что,
    // пока не выполнен bind, данное имя не пявляется в структуре ксномая. А bind у нас отложен и выполняется в отдельной функции
    // установления связей
    char* name_instance;

} remote_queue_t;
/**
  Массив (с информацией о количестве элементов) указателей на структуры с информацией о удаленных очередях (входных очередях модулей потребителей)
 */
typedef struct
{
    // Текщая длина массива
    size_t len;

    // Массив указателей на структуры содержащие информацию о очередях модулей потребителей
    remote_queue_t** remote_queues;

} sized_ar_remote_queues_t;



/**
  Структура данных, необходимых для работы со списком линков на очередь подписчика
 */
typedef struct
{
    // Входная очередь модуля подписчика
    remote_queue_t* out_queue;

    // Список полей (в составе bson объекта) которые желает получать через очередь модуль подписчик
    sized_ar_remote_obj_field_t ar_fields_of_remote_obj;

} out_queue_set_t;
/**
  Массив (с информацией о количестве элементов) указателей на структуры необходимые для работы со списком линков на очереди подписчиков
 */
typedef struct
{
    // Текщая длина массива
    size_t len;

    // Массив указателей на структуры содержащие информацию о очередях модулей потребителей
    out_queue_set_t** out_queue_sets;

} sized_ar_out_queue_set_t;



/**
  Структура данных, представляющая тип выходного объекта
  Для каждого типа выходного объекта, необходимо иметь в памяти такую обслуживающую структуру.
 */
typedef struct
{
    void* obj1;
    StatusObj status_obj1;

    void* obj2;
    StatusObj status_obj2;

    p_obj2bson obj2bson;
    p_bson2obj bson2obj;
    p_print_obj print_obj;

    // Данные необходимые для публикации объекта в разделяемую память
    shmem_set_t shmem_set;

    // Список очередей (подписчиков), в которые будет передаваться объект
    sized_ar_out_queue_set_t ar_out_queue_sets;

}out_object_t;


typedef out_object_t* (*p_get_outobj_by_outpin)(void* p_module, const char* name_out_pin, unsigned short* size_of_type, unsigned short* index_port);


typedef void (t_cycle_function)(void *cookie);
typedef t_cycle_function* p_cycle_function;
#define t_mask unsigned int


typedef struct
{
    // Указатель на подгруженную библиотеку, в которой располагается код моудля.
    // Необходимо хранить для того, чтобы при выгрузке модуля вызвать dlclose и освободить ресурсы
    void *dll_handle;

    // Имя типа модуля (например: c-gy87)
    char* module_type;

    // Имя инстанса модуля (например: c-gy87-1)
	const char* instance_name;

	/**
	 * \~english Main task
	 * \~russian
	 */
	RT_TASK task_main;
	int task_priority;


	/**
	 * \~english Transmit task
	 * \~russian
	 */
	RT_TASK task_transmit;
	RTIME transmit_task_period;

	/**
	 * \~english input queue
	 */
	RT_QUEUE in_queue;
	RTIME queue_timeout;

    // Массив содержащий информацию о входных очередях модулей потребителей
    // При инициализации данного модуля (обработке списка связей из конфигурации), данный массив
    // заполняется при помощи функции add2ar_remote_queues
    sized_ar_remote_queues_t ar_remote_queues;

    // Указатель на массив. Список типов объектов, которые пораждает модуль.
    // Массив структур инициализируется в хелпер методе инициализации в автосгенеренном для модуля коде.
    out_object_t** out_objects;

    // Указатель на бизнес-функцию (которая должна выполняться в бесконечном цикле)
	p_cycle_function func;

    /**
     * @brief mutex_obj_exchange
     * /~russian мьютех используемый для синхронизации при обмене основного потока с потоком передачи
     * в разделяемую память
     */
    RT_MUTEX mutex_obj_exchange;

    /**
     * @brief obj_cond
     * /~russian для синхронизации при обмене основного потока с потоком передачи
     * в разделяемую память
     */
    RT_COND obj_cond;

    // Указатель на структуру (входные данные модуля)
    void* input_data;

    // Функция преобразования принятого из очереди bson объекта
    // в структуру представляющую входной объект
    // Указатель на функцию должен подставляться пользовательским (автогенеренным) кодом
    p_bson2obj input_bson2obj;

    // каждый бит числа соответствцет одному из входов модуля.
    // установленный бит подразумевает запрос (пользовательского кода)
    // на обновление соответствующего биту значения во входной структуре данными
    // из разделяемой памяти
    t_mask refresh_input_mask;

    // каждый бит числа соответствцет одному из входов модуля.
    // установленный бит сигнализирует, что в текущей итерации, значение соответствующее биту обновилось
    t_mask updated_input_properties;

    // Функция в автогенеренной части модуля,
    // возвращающая метаданные выходного объекта, по имени выходного порта модуля
    p_get_outobj_by_outpin get_outobj_by_outpin;

    // Флаг показывающий (если true), что все соединения модуля установлены
    bool f_connected_links;

} module_t;


// Тип функции создания модуля
typedef module_t* (*create_f)(void *);

// Тип функции инициализации модуля
typedef int (*init_f)(module_t*, const uint8_t*, uint32_t);

// Тип функции старта модуля
typedef int (*start_f)();

// Тип функции удаления модуля
typedef void (*delete_f)(module_t*);



int init(module_t* module, const uint8_t * data, uint32_t length);

int start(void* module);

int stop(void* module);

void get_input_data(void* module);

int refresh_input(void* p_module);


char *replace(const char *s, char ch, const char *repl);


