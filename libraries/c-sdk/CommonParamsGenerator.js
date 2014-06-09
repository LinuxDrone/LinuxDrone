function make_params_structure(params_definitions) {
    var r = "";

    // формирование структуры выходного объекта
    r += "typedef struct\n";
    r += "{\n";
    params_definitions.forEach(function (param) {
        var paramName = param.name.replace(/\ /g, "_").replace(/\+/g, "");
        r += "\t" + param.type + " " + paramName + ";\n";
    });
    r += "} common_params_t;\n\n";
    return r;
}

function make_Structure2Bson(properties) {
    var r = "";
    r += "// Convert structure common_params_t to bson\n";
    r += "int common_params2bson(common_params_t* obj, bson_t* bson)\n";
    r += "{\n";
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_");
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
function make_Bson2Structure(properties) {
    var r = "";
    r += "// Convert bson to structure common_params_t\n";
    r += "int bson2common_params(void* in_module, bson_t* bson)\n";
    r += "{\n";
    r += "\tmodule_t* module = in_module;\n";
    r += "    if(!module || !bson)\n";
    r += "    {\n";
    r += "        printf(\"Error: func bson2common_params, NULL parameter\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    r += "    bson_iter_t iter;\n";
    r += "    if(!bson_iter_init (&iter, bson))\n";
    r += "    {\n";
    r += "        printf(\"Error: func bson2common_params, bson_iter_init\\n\");\n";
    r += "        return -1;\n";
    r += "    }\n\n";
    r += "    while(bson_iter_next(&iter))\n";
    r += "    {\n";
    r += "        const char* key = bson_iter_key (&iter);\n\n";
    for (var key in properties) {
        var propName = key.replace(/\ /g, "_");
        r += "        if(!strncmp(key, \"" + key + "\", XNOBJECT_NAME_LEN))\n";
        r += "        {\n";
        switch (properties[key].type) {
            case "float":
            case "double":
                r += "            if(BSON_ITER_HOLDS_DOUBLE(&iter))\n";
                r += "            {\n";
                r += "                module->common_params." + propName + " = bson_iter_double(&iter);\n";
                r += "            }\nelse ";

            case "char":
            case "short":
            case "int":
            case "long":
            case "long long":
                r += "            if(BSON_ITER_HOLDS_INT32(&iter))\n";
                r += "            {\n";
                r += "                module->common_params." + propName + " = bson_iter_int32(&iter);\n";
                r += "            }\n";
                r += "            else if(BSON_ITER_HOLDS_INT64(&iter))\n";
                r += "            {\n";
                r += "                module->common_params." + propName + " = bson_iter_int64(&iter);\n";
                r += "            }\n";
                r += "            else\n";
                r += "            {\n";
                r += "                printf(\"Unknown type for Number parameter " + propName + "\t\");\n";
                r += "            }\n";
                break;

            case "const char*":
                // TODO: Возможна проблема с тем, что строка не копируется
                r += "            uint32_t len;\n";
                r += "            module->common_params." + propName + " = bson_iter_utf8(&iter, &len);\n";
                break;

            case "bool":
                r += "            module->common_params." + propName + " = bson_iter_bool(&iter);\n";
                break;

            default:
                console.log("Unknown type " + properties[key].type + " for port " + propName);
                break;
        }
        r += "            continue;\n";
        r += "        }\n";
    }
    r += "    }\n";
    r += "    return 0;\n";
    r += "}\n\n";

    r += "// Helper function. Print structure common_params_t\n";

    r += "void print_common_params(common_params_t* obj)\n";
    r += "{\n";

    for (var key in properties) {
        var propName = key.replace(/\ /g, "_");
        switch (properties[key].type) {
            case "char":
            case "short":
            case "int":
                r += "    printf(\"" + propName + "=%i\\t\", obj->" + propName + ");\n";
                break;

            case "long":
                r += "    printf(\"" + propName + "=%i\\t\", obj->" + propName + ");\n";
                break;

            case "long long":
                r += "    printf(\"" + propName + "=%llu\\t\", obj->" + propName + ");\n";
                break;

            case "float":
            case "double":
                r += "    printf(\"" + propName + "=%lf\\t\", obj->" + propName + ");\n";
                break;

            case "const char*":
                r += "    printf(\"" + propName + "=%s\\t\", obj->" + propName + ");\n";
                break;

            case "bool":
                r += "    printf(\"" + propName + "=%s\\t\", obj->" + propName + ");\n";
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

function Create_H_file(paramsDefinitions) {
    var r = "";
    r += "#pragma once\n\n";
    r += "#include <bson.h>\n\n";
    r += make_params_structure(paramsDefinitions);
    r += "\nint common_params2bson(common_params_t* obj, bson_t* bson);\n";
    r += "int bson2common_params(void* module, bson_t* bson);\n";
    r += "void print_common_params(common_params_t* obj);\n";

    return r;
}

function Create_C_file(paramsDefinitions) {
    var r = "";

    r += "#include \"common-params.h\"\n";
    r += "#include \"module-functions.h\"\n\n";

    // Хитрожопая процедура преобразования массива объектов в объект, где каждый член - объект из массива.
    // Все танцы для того, чтобы вызвать функции создания функций bson преобразования, без их изменения.
    // И какой мудак, решил по разному определеять параметры модуля и порты модуля!
    //var paramsDefinitions = JSON.decode(JSON.encode(module.paramsDefinitions));
    var params = {};
    paramsDefinitions.forEach(function (param) {
        params[param.name] = param;
    });

    r += make_Structure2Bson(params);
    r += make_Bson2Structure(params);

    return r;
}

function main() {
    if (process.argv.length < 4) {
        console.log("Use " + process.argv[0] + " " + process.argv[1] + " path_to/ModulesCommonParams.def.js OUT_DIR");
        return -1;
    }

    var file_module_definition = process.argv[2];
    var out_dir = process.argv[3];

    var commonModuleParams = require(file_module_definition);

    var fs = require('fs');

    var paramsDefinitions = [];

    commonModuleParams.commonModuleParamsDefinition.forEach(function (param) {
        paramsDefinitions.push(param);
    });

    fs.writeFile(out_dir + "/common-params.h", Create_H_file(paramsDefinitions), function (err) {
        if (err) return console.log(err);
    });

    fs.writeFile(out_dir + "/common-params.c", Create_C_file(paramsDefinitions), function (err) {
        if (err) return console.log(err);
    });
}

module.exports.main = main;

main();

