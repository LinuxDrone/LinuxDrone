#include "test-sender.helper.h"
#include <getopt.h>


int main (int argc, char *argv[]){

    const char* short_options = "h";

    const struct option long_options[] = {
        {"help",no_argument,NULL,'h'},
        {NULL,0,NULL,0}
    };

    char* instance_name = NULL;

    int res;
    int option_index;
    while ((res=getopt_long(argc,argv,short_options, long_options,&option_index))!=-1){

        switch(res){
            case 'h':
                usage(argv);
            break;

            case '?': default:
                printf("Found unknown option\n");
            break;
        }
    }


    //printf("instance name: '%s'\n", instance_name);
    //printf("priority: %i\n", priority);
    //printf("main-task-period: %i\n", main_task_period);
    //printf("transfer-task-period: %i\n", transfer_task_period);

    return 0;
}

// Convert bson to structure params_test_sender
int argv2params_test_sender(module_t* module, int argc, char *argv[])
{
    if(!module)
    {
        printf("Error: func bson2params_test_sender, NULL parameter\n");
        return -1;
    }

    const char* short_options = "p:";
    const struct option long_options[] = {
        {"param",required_argument,NULL,'p'},
        {NULL,0,NULL,0}
    };

    int res;
    int option_index;
    while ((res=getopt_long(argc,argv,short_options, long_options,&option_index))!=-1){

        switch(res){
            case 'p':
                if (optarg!=NULL){
                    char param_name[32];
                    char param_value[32];
                    sscanf(optarg, "%s:%s", param_name, param_value);

                    params_test_sender_t* obj = (params_test_sender_t*)module->specific_params;
                    if(!strncmp(param_name, "I2C Device", XNOBJECT_NAME_LEN))
                    {
                        strcpy((char*)obj->I2C_Device, (const char *)param_value);
                        break;
                    }
                    if(!strncmp(param_name, "Test Number Param", XNOBJECT_NAME_LEN))
                    {
                        obj->Test_Number_Param = atoi(param_value);
                        break;
                    }
                    if(!strncmp(param_name, "Test Boolean Param", XNOBJECT_NAME_LEN))
                    {
                        obj->Test_Boolean_Param = atoi(param_value);
                        break;
                    }
                }
            break;


            case '?': default:
                printf("Found unknown option\n");
            break;
        }
    }

    return 0;
}


void test_sender_run (module_test_sender_t *module)
{
    printf("params for test_sender\n");
    print_params_test_sender(&module->params_test_sender);

    int cycle=0;
    while(1) {
        get_input_data(&module->module_info);

        // проверим, обновились ли данные
        if(module->module_info.updated_input_properties!=0)
        {
            // есть новые данные
        }
        else
        {
            // вышел таймаут
        }

        Output1_t* objOutput1;
        checkout_Output1(module, &objOutput1);
        objOutput1->char_out = cycle;
        objOutput1->short_out = cycle;
        objOutput1->int_out = cycle;
        objOutput1->long_out = cycle;
        objOutput1->long_long_out = cycle;
        objOutput1->float_out = cycle * 0.11;
        objOutput1->double_out = cycle * 0.23;
        char buffer_string_out [32];
        snprintf(buffer_string_out, 32, "data: %d", cycle);
        objOutput1->string_out = buffer_string_out;
//print_Output1(objOutput1);
        checkin_Output1(module, &objOutput1);

        Output2_t* objOutput2;
        checkout_Output2(module, &objOutput2);
        objOutput2->out3 = cycle;
        checkin_Output2(module, &objOutput2);



        cycle++;
    }
}
