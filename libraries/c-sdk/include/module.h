#include <native/task.h>

/**
 * \enum Reason4callback
 * \~russian Причина вызова калбека бизнес-функции
 */
typedef enum {
	/**
	 * \~russian Вышел таймаут ожидания новых данных во входной очереди
	 */
	timeout,

	/**
	 * \~russian Получены новые данные
	 */
	obtained_data
} Reason4callback;


/**
 * \typedef t_callback_business
 * \~russian Тип калбэка бизнес-функции
 */
typedef void (*t_callback_business)(Reason4callback reason);


/**
 * \fn void register_business_callback(t_callback_business callback)
 * \brief \~russian Регистрация калбэка бизнес-функции
 * \param \~russian Указатель на калбэк-функцию
 */
void register_business_callback(t_callback_business callback);

int init();

int start();
