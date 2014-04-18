function make_structures(properties, portName) {
    var r = "";
    // формирование перечисления
    r += "// Enum and Structure for port " + portName + "\n";
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
    r += "} fields_" + portName + "_t;\n\n";

    // формирование структуры выходного объекта
    r += "typedef struct\n";
    r += "{\n";
    for (var key in properties) {
        // TODO: Pay attention property type (need realize)
        r += "\tint " + key + ";\n";
    }
    r += "} " + portName + "_t;\n\n";
    return r;
}

function make_Structure2Bson(properties, portName) {
    var r = "";
    r += "// Convert structure " + portName + " to bson\n";
    r += "int " + portName + "2bson(" + portName + "_t* obj, bson_t* bson)\n";
    r += "{\n";
    for (var key in properties) {
        // TODO: Pay attention property type (need realize)
        r += "\tbson_append_int32 (bson, \"" + key + "\", -1, obj->" + key + ");\n";
    }
    r += "\treturn 0;\n";
    r += "}\n\n";

    return r;
}

function make_Bson2Structure(properties, portName) {
    var r = "";
    r += "// Convert bson to structure " + portName + "\n";
    r += "int bson2" + portName + "(module_t* module, bson_t* bson)\n";
    r += "{\n";
    var forInputCorrect = "";
    if (portName == "input") {
        forInputCorrect = " !module->input_data || ";
    }
    r += "    if(!module || " + forInputCorrect + " !bson)\n";


    r += "    {\n";
    r += "        printf(\"Error: func bson2" + portName + ", NULL parameter\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    r += "    " + portName + "_t* obj = (" + portName + "_t*)module->input_data;\n";
    r += "    bson_iter_t iter;\n";
    r += "    if(!bson_iter_init (&iter, bson))\n";
    r += "    {\n";
    r += "        printf(\"Error: func bson2" + portName + ", bson_iter_init\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    r += "    while(bson_iter_next(&iter))\n";
    r += "    {\n";
    r += "        const char* key = bson_iter_key (&iter);\n\n";
    for (var key in properties) {
        // TODO: Pay attention property type (need realize)
        r += "        if(!strncmp(key, \"" + key + "\", 100))\n";
        r += "        {\n";
        r += "            obj->" + key + " = bson_iter_int32(&iter);\n";
        r += "            module->updated_input_properties |= " + key + ";\n";
        r += "            continue;\n";
        r += "        }\n";
    }
    r += "    }\n";
    r += "    return 0;\n";
    r += "}\n\n";

    r += "// Helper function. Print structure " + portName + "\n";
    r += "void print_" + portName + "(" + portName + "_t* obj)\n";
    r += "{\n";
    for (var key in properties) {
        r += "    printf(\"" + key + "=%i\\t\", obj->" + key + ");\n";
    }
    r += "    printf(\"\\n\");\n";
    r += "}\n\n";

    return r;
}

function Create_H_file(module) {
    var module_type = module.name.replace(/-/g, "_");
    var r = "";

    r += "#pragma once\n\n";

    r += "#include \"../../../libraries/c-sdk/include/module-functions.h\"\n\n";

    if ('inputShema' in module) {
        r += make_structures(module.inputShema.properties, "input");
        //r += "\t" + input_struct + "_t input4modul;\n";
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += make_structures(out.Schema.properties, outName);
        });
    }

    // формирование структуры модуля
    r += "\n// Module Structure\n";
    r += "typedef struct {\n";
    r += "\tmodule_t module_info;\n";
    if ('inputShema' in module) {
        r += "\tinput_t input4modul;\n";
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
    r += "module_" + module_type + "_t* " + module_type + "_create();\n";
    r += "int " + module_type + "_init(module_" + module_type + "_t* module, const uint8_t* bson_data, uint32_t bson_len);\n";
    r += "int " + module_type + "_start();\n";
    r += "void " + module_type + "_delete(module_" + module_type + "_t* module);\n\n";

    return r;
}

function Create_C_file(module) {
    var module_type = module.name.replace(/-/g, "_");
    var r = "";

    r += "#include \"../include/" + module_type + ".helper.h\"\n\n";

    r += "// количество типов выходных объектов\n";
    if (module.outputs) {
        r += "#define count_outs " + module.outputs.length + "\n\n";
    }
    else {
        r += "#define count_outs 0\n\n";
    }

    r += "extern t_cycle_function " + module_type + "_run;\n\n";

    if ('inputShema' in module) {
        r += make_Bson2Structure(module.inputShema.properties, "input");
    }

    if (module.outputs) {
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");

            r += make_Structure2Bson(out.Schema.properties, outName);
            r += make_Bson2Structure(out.Schema.properties, outName);
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
                r += "    if(!strncmp(name_out_pin, \""+key+"\", 100))\n";
                r += "    {\n";
                r += "        (*offset_field) = (void*)&module->obj1_" + outName + "."+key+" - (void*)&module->obj1_" + outName + ";\n";
                r += "        (*index_port) = "+index_port+";\n";
                r += "        return &module->" + outName + ";\n";
                r += "    }\n";
                index_port++;
            };
        });
    }
    r += "    printf(\"Not found property \\\"%s\\\" among properties out objects\\n\", name_out_pin);\n";
    r += "    return NULL;\n";
    r += "}\n\n";



    r += "// Stop and delete module. Free memory.\n";
    r += "void " + module_type + "_delete(module_" + module_type + "_t* module)\n";
    r += "{\n";
    r += "    stop(module);\n";
    r += "}\n\n";


    r += "// Init module.\n";
    r += "int " + module_type + "_init(module_" + module_type + "_t* module, const uint8_t* bson_data, uint32_t bson_len)\n";
    r += "{\n";
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
    }
    r += "    module->module_info.get_outobj_by_outpin = (p_get_outobj_by_outpin)&get_outobject_by_outpin;\n";

    if ('inputShema' in module) {
        r += "    // Input\n";
        r += "    memset(&module->input4modul, 0, sizeof(input_t));\n";
        r += "    module->module_info.input_data = &module->input4modul;\n";
        r += "    module->module_info.input_bson2obj = (p_bson2obj)&bson2input;\n";
    }
    else {
        r += "    module->module_info.input_data = NULL;\n";
    }
    r += "\n";
    r += "    module->module_info.func = &" + module_type + "_run;\n\n";
    r += "    int res = init(&module->module_info, bson_data, bson_len);\n\n";
    if (module.outputs) {
        r += "    // для каждого типа порождаемого объекта инициализируется соответсвующая структура\n";
        r += "    // и указываются буферы (для обмена данными между основным и передающим потоком)\n";
        module.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "    // " + outName + "\n";
            r += "    init_object_set(&module->" + outName + ", module->module_info.instance_name, \"" + outName + "\");\n";
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
            r += "    return checkout4writer(module, &module->" + outName + ", obj);\n";
            r += "}\n\n";


            r += "/**\n";
            r += "* @brief checkin4writer_" + outName + "\n";
            r += "* /~ Возвращает объект системе (данные будут переданы в разделяемую память)\n";
            r += "* @param obj\n";
            r += "* @return\n";
            r += "*/\n";
            r += "int checkin_" + outName + "(module_" + module_type + "_t* module, " + outName + "_t** obj)\n";
            r += "{\n";
            r += "    return checkin4writer(module, &module->" + outName + ", obj);\n";
            r += "}\n\n";
        });
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
        var module_type = module.name.replace(/-/g, "_");

        var text_H_file = Create_H_file(module);
        //console.log(text_H_file);
        fs.writeFile(out_dir + "/include/" + module_type + ".helper.h", text_H_file, function (err) {
            if (err) return console.log(err);
        });

        var text_C_file = Create_C_file(module);
        //console.log(text_H_file);
        fs.writeFile(out_dir + "/src/" + module_type + ".helper.c", text_C_file, function (err) {
            if (err) return console.log(err);
        });
    });
}

module.exports.main = main;

main();

