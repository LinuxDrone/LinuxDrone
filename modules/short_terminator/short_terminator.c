#include "short_terminator.helper.h"

void short_terminator_run (module_short_terminator_t *module)
{
    while(1) {
        get_input_data((module_t*)module);
    }
}
