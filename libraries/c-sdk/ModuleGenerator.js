function make_structures(properties, portName)
{
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

function Create_H_file(module) {
    var module_type = module.name.replace(/-/g,"_");
    var r = "";

    r += "#ifndef GENERATED_CODE_H_\n";
    r += "#define GENERATED_CODE_H_\n\n";

    r += "#include \"../../../libraries/c-sdk/include/module-functions.h\"\n\n";

    if ('inputShema' in module) {
        r += make_structures(module.inputShema.properties, "input");
        //r += "\t" + input_struct + "_t input4modul;\n";
    }

    module.outputs.forEach(function (out) {
        var outName = out.name.replace(/\+/g, "");
        r += make_structures(out.Schema.properties, outName);
    });


    // формирование структуры модуля
    r += "\n// Module Structure\n";
    r += "typedef struct {\n";
    r += "\tmodule_t module_info;\n";
    if ('inputShema' in module) {
        r += "\tinput_t input4modul;\n";
    }
    module.outputs.forEach(function (out) {
        var outName = out.name.replace(/\+/g, "");
        r += "\n\t// набор данных для выхода "+outName+"\n";
        r += "\tshmem_publisher_set_t  " + outName + ";\n";
        r += "\t" + outName + "_t obj1_" + outName + ";\n";
        r += "\t" + outName + "_t obj2_" + outName + ";\n";
    });
    r += "} module_" + module_type + "_t;\n\n";


    // Формирование объявлений функций
    r += "\n// Helper functions\n";
    r += "module_" + module_type + "_t* " + module_type + "_create();\n";
    r += "int " + module_type + "_init(module_" + module_type + "_t* module, const uint8_t* bson_data, uint32_t bson_len);\n";
    r += "int " + module_type + "_start();\n";
    r += "void " + module_type + "_delete(module_" + module_type + "_t* module);\n\n";
    r += "#endif\n";

    return r;
}

function main() {
    if (process.argv.length < 3) {
        console.log("Use " + process.argv[0] + " " + process.argv[1] + " YOUR_MODULE.def.json");
        return -1;
    }

    var file_module_definition = process.argv[2];
    var fs = require('fs');

    fs.readFile(file_module_definition, 'utf8', function (err, data) {
        if (err) {
            console.log('Error: ' + err);
            return;
        }

        var module = JSON.parse(data);
        //console.dir(module);

        var text_H_file = Create_H_file(module);
        //console.log(text_H_file);

        fs.writeFile('generated_code.h', text_H_file, function (err) {
            if (err) return console.log(err);
        });

    });
}

main();