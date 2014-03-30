#include <native/task.h>
#include <bson.h>

/**
 * \enum Reason4callback
 * \~russian ������� ������ ������� ������-�������
 */
typedef enum {
	/**
	 * \~russian ����� ������� �������� ����� ������ �� ������� �������
	 */
	timeout,

	/**
	 * \~russian �������� ����� ������
	 */
	obtained_data
} Reason4callback;


/**
 * \typedef t_callback_business
 * \~russian ��� ������� ������-�������
 */
typedef void (*t_callback_business)(Reason4callback reason);


/**
 * \fn void register_business_callback(t_callback_business callback)
 * \brief \~russian ����������� ������� ������-�������
 * \param \~russian ��������� �� ������-�������
 */
void register_business_callback(t_callback_business callback);

int init(const uint8_t * data, uint32_t length);

int start();
