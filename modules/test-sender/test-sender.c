#include "test-sender.helper.h"
#include <getopt.h>


int main (int argc, char *argv[]){

    const char* short_options = "n:r::m::t::iop";

    const struct option long_options[] = {
        {"help",no_argument,NULL,'h'},
        {"name",required_argument,NULL,'n'},
        {"priority",optional_argument,NULL,'r'},
        {"main-task-period",optional_argument,NULL,'m'},
        {"transfer-task-period",optional_argument,NULL,'t'},
        {"in-link",required_argument,NULL,'i'},
        {"out-link",required_argument,NULL,'o'},
        {"param",required_argument,NULL,'p'},
        {NULL,0,NULL,0}
    };

    char* instance_name = NULL;
    int priority = 80;
    int main_task_period = 20000;
    int transfer_task_period = 20000;


    int res;
    int option_index;
    while ((res=getopt_long(argc,argv,short_options, long_options,&option_index))!=-1){

        switch(res){
            case 'h':
                usage(argv);
            break;

            case 'n':
                if (optarg!=NULL) instance_name = optarg;
            break;

            case 'r':
                if (optarg!=NULL)
                {
                    priority = atoi(optarg);
                    if(priority < 1 || priority > 99)
                    {
                        printf("argument 'priority' valid values in the range 1-99\n\n");
                        usage(argv);
                    }
                }
            break;

            case 'm':
                if (optarg!=NULL)
                {
                    main_task_period = atoi(optarg);
                    if(main_task_period < 0)
                    {
                        printf("argument 'main-task-period' valid values >-1\n\n");
                        usage(argv);
                    }
                }
            break;

            case 't':
                if (optarg!=NULL)
                {
                    transfer_task_period = atoi(optarg);
                    if(transfer_task_period < 0)
                    {
                        printf("argument 'transfer-task-period' valid values >-1\n\n");
                        usage(argv);
                    }
                }
            break;

            case '?': default:
                printf("Found unknown option\n");
            break;
        }
    }

    if(instance_name==NULL){
        printf("required argument --name\n\n");
        usage(argv);
    }

    printf("instance name: '%s'\n", instance_name);
    printf("priority: %i\n", priority);
    printf("main-task-period: %i\n", main_task_period);
    printf("transfer-task-period: %i\n", transfer_task_period);

    return 0;
}

usage(char *argv[])
{
    fprintf(stderr, "usage: %s \n", argv[0]);

    fprintf(stderr, "-n, --name=NAME\n");
    fprintf(stderr, "\trequired argument\n");
    fprintf(stderr, "\tInstance name\n\n");

    fprintf(stderr, "-r, --priority=PRIORITY\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tMain realtime thread priority (1-99, default: 80)\n\n");

    fprintf(stderr, "-m, --main-task-period=PERIOD\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tBusiness function execution period in microseconds (default: 20000)\n\n");

    fprintf(stderr, "-t, --transfer-task-period=PERIOD\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tOutput data to shared memory copy period in microseconds (default: 20000)\n\n");

    fprintf(stderr, "-i, --in-link=INSTANCE2.OUT1->IN1\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tInput link (provides data from another instance to this one through shared memory)\n\n");

    fprintf(stderr, "-o, --out-link=OUT1->INSTANCE3.IN1\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tOutput link (provides data from this instance to another one through pipe)\n\n");

    fprintf(stderr, "-p, --param=PARAM:VALUE\n");
    fprintf(stderr, "\toptional\n");
    fprintf(stderr, "\tSetting a parameter\n\n");

    exit(EXIT_FAILURE);
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
