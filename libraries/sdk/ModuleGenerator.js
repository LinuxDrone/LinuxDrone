function make_params_structure(params_definitions, moduleName) {
    moduleName = moduleName.replace(/-/g, "_");
    var r = "";
    // формирование перечисления
    r += "// Enum and Structure params for " + moduleName + "\n";
    r += "typedef enum\n";
    r += "{\n";
    var i = 0;
    params_definitions.forEach(function (param) {
        var paramName = param.name.replace(/\ /g, "_").replace(/\+/g, "").replace(/-/g, "_");
        if (i != 0) {
            r += ",\n";
        }
        r += "\t" + paramName + " =\t0b";
        for (var c = 0; c < 32; c++) {
            if (i == 31 - c) {
                r += "1";
            }
            else {
                r += "0";
            }
        }
        i++;
    });

    r += "\n";
    r += "} fields4params_" + moduleName + "_t;\n\n";

    // формирование структуры выходного объекта
    r += "typedef struct\n";
    r += "{\n";
    params_definitions.forEach(function (param) {
        var paramName = param.name.replace(/\ /g, "_").replace(/\+/g, "").replace(/-/g, "_");
        r += "\t" + param.type + " " + paramName + ";\n";
    });
    r += "} params_" + moduleName + "_t;\n\n";
    return r;
}

function make_structures(properties, outName) {
    var r = "";
    // формирование перечисления
    r += "// Enum and Structure for port " + outName + "\n";
    r += "typedef enum\n";
    r += "{\n";
    var i = 0;
    for (var key in properties) {
        if (i != 0) {
            r += ",\n";
        }
        r += "\t" + key + " =\t0b";
        for (var c = 0; c < 32; c++) {
            if (i == 31 - c) {
                r += "1";
            }
            else {
                r += "0";
            }
        }
        i++;
    }
    r += "\n";
    r += "} fields_" + outName + "_t;\n\n";

    // формирование структуры выходного объекта
    r += "typedef struct\n";
    r += "{\n";
    for (var key in properties) {
        r += "\t" + properties[key].type + " " + key + ";\n";
    }
    r += "} " + outName + "_t;\n\n";
    return r;
}

function make_Structure2Bson(properties, outName) {
    var r = "";
    r += "// Convert structure " + outName + " to bson\n";
    r += "int " + outName + "2bson(" + outName + "_t* obj, bson_t* bson)\n";
    r += "{\n";
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_").replace(/-/g, "_");
        switch (properties[key].type) {
            case "char":
            case "short":
            case "int":
            case "long":
                r += "\tbson_append_int32 (bson, \"" + key + "\", -1, obj->" + propName + ");\n";
                break;

            case "long long":
                r += "\tbson_append_int64 (bson, \"" + key + "\", -1, obj->" + propName + ");\n";
                break;

            case "float":
            case "double":
                r += "\tbson_append_double (bson, \"" + key + "\", -1, obj->" + propName + ");\n";
                break;

            case "const char*":
                // TODO: Возможна проблема с тем, что строка не копируется
                r += "\tbson_append_utf8 (bson, \"" + key + "\", -1, obj->" + propName + ", -1);\n";
                break;

            case "bool":
                r += "\tbson_append_bool(bson, \"" + key + "\", -1, obj->" + propName + ");\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + key);
                break;
        }
    }
    r += "\treturn 0;\n";
    r += "}\n\n";

    return r;
}

/**
 * param set_update_fact Отражать факт обновления свойства или нет.
 * Для данных обмена между модулями установка данного факта имеет смысл.
 * Для обновления параметра - пока не понятно зачем.
 * Так как данная функция используется для генерации функций как для преобразования параметров, так и для данных обмена
 * то при помощи данного параметра будем указывать, когда необходимо отражать факт изменения.
 */
function make_Bson2Structure(properties, outName, module_type, set_update_fact) {
    var r = "";
    r += "// Convert bson to structure " + outName + "\n";
    r += "int bson2" + outName + "(module_t* module, bson_t* bson)\n";
    r += "{\n";
    var forInputCorrect = "";
    if (outName == "input") {
        forInputCorrect = " !module->input_data || ";
    }
    r += "    if(!module || " + forInputCorrect + " !bson)\n";


    r += "    {\n";
    r += "        printf(\"Error: func bson2" + outName + ", NULL parameter\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    if (set_update_fact) {
        r += "    " + outName + "_t* obj = (" + outName + "_t*)module->input_data;\n";
    } else {
        r += "    " + outName + "_t* obj = (" + outName + "_t*)module->specific_params;\n";
    }
    r += "    bson_iter_t iter;\n";
    r += "    if(!bson_iter_init (&iter, bson))\n";
    r += "    {\n";
    r += "        printf(\"Error: func bson2" + outName + ", bson_iter_init\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    r += "    while(bson_iter_next(&iter))\n";
    r += "    {\n";
    r += "        const char* key = bson_iter_key (&iter);\n\n";
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_").replace(/-/g, "_");
        r += "        if(!strncmp(key, \"" + key + "\", XNOBJECT_NAME_LEN))\n";
        r += "        {\n";
        switch (properties[key].type) {
            case "float":
            case "double":
                r += "            if(BSON_ITER_HOLDS_DOUBLE(&iter))\n";
                r += "            {\n";
                r += "                obj->" + propName + " = bson_iter_double(&iter);\n";
                r += "            }\nelse ";

            case "char":
            case "short":
            case "int":
            case "long":
            case "long long":
                r += "            if(BSON_ITER_HOLDS_INT32(&iter))\n";
                r += "            {\n";
                r += "                obj->" + propName + " = bson_iter_int32(&iter);\n";
                r += "            }\n";
                r += "            else if(BSON_ITER_HOLDS_INT64(&iter))\n";
                r += "            {\n";
                r += "                obj->" + propName + " = bson_iter_int64(&iter);\n";
                r += "            }\n";
                r += "            else\n";
                r += "            {\n";
                r += "                printf(\"Unknown type for Number parameter " + propName + "\t\");\n";
                r += "            }\n";
                break;

            case "const char*":
                // TODO: Возможна проблема с тем, что строка не копируется
                r += "            uint32_t len;\n";
                r += "            obj->" + propName + " = bson_iter_utf8(&iter, &len);\n";
                break;

            case "bool":
                r += "            obj->" + propName + " = bson_iter_bool(&iter);\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + propName);
                break;
        }
        if (set_update_fact) {
            r += "            module->updated_input_properties |= " + propName + ";\n";
        }
        r += "            continue;\n";
        r += "        }\n";
    }
    r += "    }\n";
    r += "    return 0;\n";
    r += "}\n\n";

    r += "// Helper function. Print structure " + outName + "\n";
    if (outName == "input") {
        r += "void print_" + module_type + "(void* obj1)\n";
        r += "{\n";
        r += "    " + outName + "_t* obj=obj1;\n";
    } else {
        r += "void print_" + outName + "(" + outName + "_t* obj)\n";
        r += "{\n";
    }
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_").replace(/-/g, "_");
        switch (properties[key].type) {
            case "char":
            case "short":
            case "int":
            case "long":
            case "long long":
                r += "    printf(\"" + propName + "=%i\\t\", obj->" + propName + ");\n";
                break;

            case "float":
            case "double":
                r += "    printf(\"" + propName + "=%lf\\t\", obj->" + propName + ");\n";
                break;

            case "const char*":
                r += "    printf(\"" + propName + "=%s\\t\", obj->" + propName + ");\n";
                break;

            case "bool":
                r += "    printf(\"" + propName + "=%i\\t\", obj->" + propName + ");\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + propName);
                break;
        }
    }
    r += "    printf(\"\\n\");\n";
    r += "}\n\n";
    return r;
}

function make_print_help(params){
    var r = "// Print help\n";
    r += "void print_help()\n";
    r += "{\n";
    for (var key in params) {
        var paramName = key.replace(/\ /g, "_");
        var param = params[key];

        r += "    fprintf(stderr, \"--"+paramName+"="+param.type.toUpperCase()+" VALUE\\n\");\n";
        if('defaultValue' in param){
            r += "    fprintf(stderr, \"\\toptional\\n\");\n";
        }else{
            r += "    fprintf(stderr, \"\\trequired\\n\");\n";
        }

        if('description' in param){
            if('en' in param.description){
                r += "    fprintf(stderr, \"\\t"+param.description.en;
                if('validValues' in param || 'defaultValue' in param){
                    r += " (";
                    if('validValues' in param){
                        r += param.validValues;
                        if('defaultValue' in param){
                            r += " ";
                        }
                    }
                    if('defaultValue' in param){
                        r += "default: " + param.defaultValue;
                    }
                    r += ")";
                }
                r += "\\n\\n\");\n\n";
            }
        }
    }
    r += "}\n\n";
    return r;
}

function make_argv2Structure(properties, module_type) {
    var structureName = "params_" + module_type;
    var r = "";
    r += "// Convert argv to structure " + structureName + "\n";
    r += "int argv2" + structureName + "(module_t* module, int argc, char *argv[])\n";
    r += "{\n";
    r += "    if(!module)\n";
    r += "    {\n";
    r += "        printf(\"Error: func argv2" + structureName + ", NULL parameter\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";

    r += "    const char* short_options = \"\";\n";
    r += "    const struct option long_options[] = {\n";
    var ip=100;
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_");
        r += "        {\""+propName+"\",required_argument,NULL,"+ip+"},\n";
        ip++;
    }
    r += "        {NULL,0,NULL,0}\n";
    r += "    };\n\n";

    r += "    int res;\n";
    r += "    int option_index;\n";
    r += "    opterr=0;\n";
    r += "    optind=0;\n";
    r += "    while ((res=getopt_long(argc,argv,short_options, long_options,&option_index))!=-1){\n";
    r += "                    " + structureName + "_t* obj = (" + structureName + "_t*)module->specific_params;\n";

    r += "        if (optarg!=NULL){\n";
    r += "            switch(res){\n";
    ip=100;
    for (var key in properties) {

        r += "                case "+ip+":\n";
        var propName = key.replace(/\ /g, "_").replace(/-/g, "_");
        switch (properties[key].type) {
            case "float":
            case "double":
                r += "                    obj->" + propName + " = atof(optarg);\n";

            case "char":
            case "short":
            case "int":
            case "bool":
                r += "                    obj->" + propName + " = atoi(optarg);\n";
                break;

            case "long":
                r += "                    obj->" + propName + " = atol(optarg);\n";
                break;

            case "long long":
                r += "                    obj->" + propName + " = atoll(optarg);\n";
                break;

            case "const char*":
                // TODO: Возможна проблема с тем, что строку надо освобождать
                r += "                    obj->" + propName + " = malloc(strlen(optarg)+1);\n";
                r += "                    strcpy((char*)obj->" + propName + ", optarg);\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + propName);
                break;
        }
        r += "                break;\n\n";
        ip++;
    }
    r += "            }\n";
    r += "        }\n";
    r += "    }\n";
    r += "    return 0;\n";
    r += "}\n\n";


    r += "// Helper function. Print structure " + structureName + "\n";

    r += "void print_" + structureName + "(" + structureName + "_t* obj)\n";
    r += "{\n";
    r += "    fprintf(stdout, \"\\nSpecific params:\\n\");\n";

    for (var key in properties) {
        var propName = key.replace(/\ /g, "_").replace(/-/g, "_");
        switch (properties[key].type) {
            case "char":
            case "short":
            case "int":
            case "long":
            case "long long":
                r += "    fprintf(stdout, \"\\t" + key + ": %i\\n\", obj->" + propName + ");\n";
                break;

            case "float":
            case "double":
                r += "    fprintf(stdout, \"\\t" + key + ": %lf\\n\", obj->" + propName + ");\n";
                break;

            case "const char*":
                r += "    fprintf(stdout, \"\\t" + key + ": %s\\n\", obj->" + propName + ");\n";
                break;

            case "bool":
                r += "    fprintf(stdout, \"\\t" + key + ": %i\\n\", obj->" + propName + ");\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + propName);
                break;
        }
    }
    r += "    fprintf(stdout, \"\\n\");\n";
    r += "}\n\n";

    return r;
}

function Create_H_file(module) {
    var module_type = module.name.replace(/-/g, "_");
    var r = "";

    r += "#pragma once\n\n";

    r += "#include \"module-functions.h\"\n\n";

    r += "\n#ifdef __cplusplus\n";
    r += "extern \"C\" {\n"
    r += "#endif\n\n";

    if ('inputShema' in module) {
        r += make_structures(module.inputShema.properties, "input");
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += make_structures(out.Schema.properties, outName);
        });
    }

    if ('paramsDefinitions' in module) {
        r += make_params_structure(module.paramsDefinitions, module.name);
    }

    // формирование структуры модуля
    r += "\n// Module Structure\n";
    r += "typedef struct {\n";
    r += "\tmodule_t module_info;\n";
    if ('inputShema' in module) {
        r += "\tinput_t input4module;\n";
    }

    if ('paramsDefinitions' in module) {
        r += "\n\t// Настроечные параметры\n";
        r += "\tparams_" + module_type + "_t params_" + module_type + ";\n";
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "\n\t// набор данных для выхода " + outName + "\n";
            r += "\tout_object_t  " + outName + ";\n";
            r += "\t" + outName + "_t obj1_" + outName + ";\n";
            r += "\t" + outName + "_t obj2_" + outName + ";\n";
        });
    }
    r += "} module_" + module_type + "_t;\n\n";


    // Формирование объявлений функций
    r += "\n// Helper functions\n";
    r += "module_" + module_type + "_t* " + module_type + "_create(void *handle);\n";
    r += "int " + module_type + "_init(module_" + module_type + "_t* module, int argc, char *argv[]);\n";
    r += "int " + module_type + "_start(module_" + module_type + "_t* module);\n";
    r += "void " + module_type + "_delete(module_" + module_type + "_t* module);\n\n";

    if ('paramsDefinitions' in module) {
        r += "/**\n";
        r += "* @brief \\~russian Заполняет указатель адресом на структуру, содержащую настроечные параметры модуля\n";
        r += "* @param module\n";
        r += "* @param params\n";
        r += "* @return\n";
        r += "*/\n";
        r += "int checkout_params_" + module_type + "(module_t* module, void** params);\n\n";

        r += "/**\n";
        r += "* @brief \\~russian Освобождает мьютекс, удерживаемый на время работы со структурой параметров\n";
        r += "* @param module\n";
        r += "* @return\n";
        r += "*/\n";
        r += "int checkin_params_" + module_type + "(module_t* module);\n\n";
        if (module.outputs) {
            module.outputs.forEach(function (out) {
                var outName = out.name.replace(/\+/g, "");
                r += "int checkout_" + outName + "(module_" + module_type + "_t* module, " + outName + "_t** obj);\n";
                r += "int checkin_" + outName + "(module_" + module_type + "_t* module, " + outName + "_t** obj);\n\n";
            });
        }
    }

    // Формирование перечисления типов команд
    if ('commands' in module) {
        r += "// Enum of command types\n";
        r += "typedef enum\n";
        r += "{\n";
        var i = 0;
        module.commands.forEach(function (cmd) {
            var cmdName = cmd.name.replace(/\-/g, "_");
            if (i != 0) {
                r += ",\n";
            }
            r += "    cmd_" + cmdName;
            i++;
        });
        r += "\n} " + module_type + "_command_t;\n";
    }

    r += "\n#ifdef __cplusplus\n";
    r += "}\n"
    r += "#endif\n";

    return r;
}

function Create_C_file(module) {
    var module_type = module.name;
    var r = "";

    r += "#include \"" + module_type + ".helper.h\"\n";
    r += "#include <getopt.h>\n\n";

    r += "// Определение модуля в JSON\n";
    r += "const char* m_definition = \"" + JSON.stringify(module).replace(/"/g, "\\\"") + "\";\n\n";

    module_type = module_type.replace(/-/g, "_");

    r += "int checkout4writer(module_t* module, out_object_t* set, void** obj);\n";
    r += "int checkin4writer(module_t* module, out_object_t* set, void** obj);\n\n";
    r += "// количество типов выходных объектов\n";
    if (module.outputs) {
        r += "#define count_outs " + module.outputs.length + "\n\n";
    }
    else {
        r += "#define count_outs 0\n\n";
    }
    r += "extern t_cycle_function " + module_type + "_run;\n";

    if ('commands' in module) {
        r += "extern t_cmd_function " + module_type + "_command;\n";
    }
    r += "\n";



    if ('inputShema' in module) {
        r += "int get_inputmask_by_inputname(char* input_name)\n";
        r += "{\n";
        for (var key in module.inputShema.properties) {
            r += "    if(!strncmp(input_name, \"" + key + "\", XNOBJECT_NAME_LEN))\n";
            r += "        return " + key + ";\n";
        }
        r += "\n    return 0;\n";
        r += "}\n\n";
        r += make_Bson2Structure(module.inputShema.properties, "input", module_type, true);
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += make_Structure2Bson(out.Schema.properties, outName);
            r += make_Bson2Structure(out.Schema.properties, outName, "", true);
        });
    }


    r += "// Create module.\n";
    r += "module_" + module_type + "_t* " + module_type + "_create(void *handle)\n";
    r += "{\n";
    r += "    module_" + module_type + "_t* module = calloc(1, sizeof(module_" + module_type + "_t));\n";
    r += "    // Сохраним указатель на загруженную dll\n";
    r += "    module->module_info.dll_handle = handle;\n";
    r += "    module->module_info.out_objects = calloc(count_outs+1, sizeof(void *));\n";
    if (module.outputs) {
        module.outputs.forEach(function (out, i) {
            var outName = out.name.replace(/\+/g, "");
            r += "    module->module_info.out_objects[" + i + "]=&module->" + outName + ";\n";
        });
    }
    r += "    return module;\n";
    r += "}\n\n";


    if (module.outputs) {
        r += "// Возвращает указатель на структуру выходного объекта, по имени пина\n";
        r += "// Используется при подготовке списка полей, для мапинга объектов (для передачи в очередь)\n";
        r += "out_object_t* get_outobject_by_outpin(module_" + module_type + "_t* module, char* name_out_pin, unsigned short* offset_field, unsigned short* index_port)\n";
        r += "{\n";
        r += "    (*offset_field) = 0;\n";
        r += "    (*index_port) = 0;\n";
        if (module.outputs) {
            module.outputs.forEach(function (out) {
                var outName = out.name.replace(/\+/g, "");
                var index_port = 0;
                for (var key in out.Schema.properties) {
                    r += "    if(!strncmp(name_out_pin, \"" + key + "\", XNOBJECT_NAME_LEN))\n";
                    r += "    {\n";
                    r += "        (*offset_field) = (void*)&module->obj1_" + outName + "." + key + " - (void*)&module->obj1_" + outName + ";\n";
                    r += "        (*index_port) = " + index_port + ";\n";
                    r += "        return &module->" + outName + ";\n";
                    r += "    }\n";
                    index_port++;
                }
                ;
            });
        }
        r += "    printf(\"Not found property \\\"%s\\\" among properties out objects\\n\", name_out_pin);\n";
        r += "    return NULL;\n";
        r += "}\n\n";
    }

    if ('inputShema' in module) {
        r += "int get_offset_in_input_by_inpinname(module_" + module_type + "_t* module, char* name_inpin)\n";
        r += "{\n";
        for (var key in module.inputShema.properties) {
            r += "    if(!strncmp(name_inpin, \"" + key + "\", XNOBJECT_NAME_LEN))\n";
            r += "    {\n";
            r += "        return (void*)&module->input4module." + key + " - (void*)&module->input4module;\n";
            r += "    }\n";
        }
        r += "    printf(\"Not found property \\\"%s\\\" among properties in input object\\n\", name_inpin);\n";
        r += "    return -1;\n";
        r += "}\n\n";
    }


    r += "// Stop and delete module. Free memory.\n";
    r += "void " + module_type + "_delete(module_" + module_type + "_t* module)\n";
    r += "{\n";
    r += "    stop(module);\n";
    r += "}\n\n";


    // Хитрожопая процедура преобразования массива объектов в объект, где каждый член - объект из массива.
    // Все танцы для того, чтобы вызвать функции создания функций bson преобразования, без их изменения.
    // И какой мудак, решил по разному определеять параметры модуля и порты модуля!
    //var paramsDefinitions = JSON.decode(JSON.encode(module.paramsDefinitions));
    var params = {};
    module.paramsDefinitions.forEach(function (param) {
        params[param.name] = param;
    });

    if ('paramsDefinitions' in module) {
        r += make_Structure2Bson(params, "params_" + module_type);
        //r += make_Bson2Structure(params, "params_" + module_type, "", false);
        r += make_argv2Structure(params, module_type);
        r += make_print_help(params);
    }


    r += "// Init module.\n";
    r += "int " + module_type + "_init(module_" + module_type + "_t* module, int argc, char *argv[])\n";
    r += "{\n";

    if ('paramsDefinitions' in module) {
        r += "    // Настроечные параметры\n";
        r += "    module->module_info.specific_params = &module->params_" + module_type + ";\n";
        r += "    module->module_info.params2bson = (p_obj2bson)&params_" + module_type + "2bson;\n";
        r += "    module->module_info.argv2params = (p_argv2obj)&argv2params_" + module_type + ";\n";
        r += "    module->module_info.print_params = (p_print_obj)&print_params_" + module_type + ";\n\n";
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "    // " + outName + "\n";
            r += "    // временное решение для указания размера выделяемой памяти под bson  объекты каждого типа\n";
            r += "    // в реальности должны один раз создаваться тестовые bson объекты, вычисляться их размер и передаваться в функцию инициализации\n";
            r += "    module->" + outName + ".shmem_set.shmem_len = 300;\n";
            r += "    module->" + outName + ".obj1 = &module->obj1_" + outName + ";\n";
            r += "    module->" + outName + ".obj2 = &module->obj2_" + outName + ";\n";
            r += "    module->" + outName + ".obj2bson = (p_obj2bson)&" + outName + "2bson;\n";
            r += "    module->" + outName + ".bson2obj = (p_bson2obj)&bson2" + outName + ";\n";
            r += "    module->" + outName + ".print_obj = (p_print_obj)&print_" + outName + ";\n\n";
        });
        r += "    module->module_info.get_outobj_by_outpin = (p_get_outobj_by_outpin)&get_outobject_by_outpin;\n";
    }


    if ('inputShema' in module) {
        r += "    // Input\n";
        r += "    memset(&module->input4module, 0, sizeof(input_t));\n";
        r += "    module->module_info.input_data = &module->input4module;\n";
        r += "    module->module_info.input_bson2obj = (p_bson2obj)&bson2input;\n";
        r += "    module->module_info.get_inmask_by_inputname = (p_get_inputmask_by_inputname)&get_inputmask_by_inputname;\n";
        r += "    module->module_info.get_offset_in_input_by_inpinname = (p_get_offset_in_input_by_inpinname)&get_offset_in_input_by_inpinname;\n";
    }
    else {
        r += "    module->module_info.input_data = NULL;\n";
    }
    r += "\n";

    r += "    // Строка с JSON определением модуля.\n";
    r += "    module->module_info.json_module_definition = m_definition;\n\n";

    r += "    // ССылка на функцию выводящую в консоль справку по параметрам запсука модуля.\n";
    r += "    module->module_info.print_help = (p_print_help)&print_help;\n\n";

    r += "    // Сохранение ссылки на бизнес-функцию.\n";
    r += "    module->module_info.func = &" + module_type + "_run;\n";


    // Формирование перечисления типов команд
    if ('commands' in module) {
        r += "    // Сохранение ссылки на функцию-обработчик команды.\n";
        r += "    module->module_info.cmd_func = &" + module_type + "_command;\n";
    }else{
        r += "    // В определении модуля нет команд, а значит нет и фунции их обработки.\n";
        r += "    module->module_info.cmd_func = NULL;\n";
    }
    r += "\n";


    if ('inputShema' in module) {
        r += "    module->module_info.print_input = &print_" + module_type + ";\n\n";
    }
    r += "    int res = init(&module->module_info, argc, argv);\n\n";
    if (module.outputs) {
        r += "    // для каждого типа порождаемого объекта инициализируется соответсвующая структура\n";
        r += "    // и указываются буферы (для обмена данными между основным и передающим потоком)\n";
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "    // " + outName + "\n";
            r += "    init_object_set(&module->" + outName + ".shmem_set, module->module_info.instance_name, \"" + outName + "\");\n";
        });
    }
    r += "\n    return res;\n";
    r += "}\n\n";


    r += "int " + module_type + "_start(module_" + module_type + "_t* module)\n";
    r += "{\n";
    r += "    if (start(module) != 0)\n";
    r += "        return -1;\n";
    r += "    return 0;\n";
    r += "}\n\n";


    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "/**\n";
            r += "* @brief checkout4writer_" + outName + "\n";
            r += "* /~russian    Заполняет указатель адресом на структуру " + outName + "_t,\n";
            r += "*              которую можно заполнять данными для последующей передачи в разделяемую память\n";
            r += "* @param obj\n";
            r += "* @return\n";
            r += "* /~russian 0 в случае успеха\n";
            r += "*/\n";
            r += "int checkout_" + outName + "(module_" + module_type + "_t* module, " + outName + "_t** obj)\n";
            r += "{\n";
            r += "\treturn checkout4writer(&module->module_info, &module->" + outName + ", (void**)obj);\n";
            r += "}\n\n";


            r += "/**\n";
            r += "* @brief checkin4writer_" + outName + "\n";
            r += "* /~ Возвращает объект системе (данные будут переданы в разделяемую память)\n";
            r += "* @param obj\n";
            r += "* @return\n";
            r += "*/\n";
            r += "int checkin_" + outName + "(module_" + module_type + "_t* module, " + outName + "_t** obj)\n";
            r += "{\n";
            r += "\treturn checkin4writer(&module->module_info, &module->" + outName + ", (void**)obj);\n";
            r += "}\n\n";
        });
    }

    if ('paramsDefinitions' in module) {
        r += "/**\n";
        r += "* @brief \\~russian Заполняет указатель адресом на структуру, содержащую настроечные параметры модуля\n";
        r += "* @param module\n";
        r += "* @param params\n";
        r += "* @return\n";
        r += "*/\n";
        r += "int checkout_params_" + module_type + "(module_t* module, void** params)\n";
        r += "{\n";
        r += "  int res = rt_mutex_acquire(&module->mutex_obj_exchange, TM_INFINITE);\n";
        r += "  if (res != 0)\n";
        r += "  {\n";
        r += "      printf(\"error checkout_params_" + module_type + ": rt_mutex_acquire\\n\");\n";
        r += "      return res;\n";
        r += "  }\n";
        r += "  *params = module->specific_params;\n";
        r += "  return 0;\n";
        r += "}\n\n";

        r += "/**\n";
        r += "* @brief \\~russian Освобождает мьютекс, удерживаемый на время работы со структурой параметров\n";
        r += "* @param module\n";
        r += "* @return\n";
        r += "*/\n";
        r += "int checkin_params_" + module_type + "(module_t* module)\n";
        r += "{\n";
        r += "  int res = rt_mutex_release(&module->mutex_obj_exchange);\n";
        r += "  if (res != 0)\n";
        r += "  {\n";
        r += "      printf(\"error checkin_params_" + module_type + ":  rt_mutex_release\\n\");\n";
        r += "      return res;\n";
        r += "  }\n";
        r += "  return 0;\n";
        r += "}\n\n";

    }

    return r;
}

function main() {
    if (process.argv.length < 4) {
        console.log("Use " + process.argv[0] + " " + process.argv[1] + " YOUR_MODULE.def.json OUT_DIR");
        return -1;
    }

    var file_module_definition = process.argv[2];
    var out_dir = process.argv[3];
    var fs = require('fs');

    fs.readFile(file_module_definition, 'utf8', function (err, data) {
        if (err) {
            console.log('Error: ' + err);
            return;
        }

        var module = JSON.parse(data);
        var module_type = module.name;

        //commonModuleParams.commonModuleParamsDefinition.forEach(function (param) {
        //  module.paramsDefinitions.push(param);
        //});

        var text_H_file = Create_H_file(module);
        //console.log(text_H_file);
        fs.writeFile(out_dir + "/" + module_type + ".helper.h", text_H_file, function (err) {
            if (err) return console.log(err);
        });

        var text_C_file = Create_C_file(module);
        //console.log(text_H_file);
        fs.writeFile(out_dir + "/" + module_type + ".helper.c", text_C_file, function (err) {
            if (err) return console.log(err);
        });
    });
}

module.exports.main = main;

main();

