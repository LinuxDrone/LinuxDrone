function Create_H_file(module)
{
    var r = "";

    r += "#ifndef GENERATED_CODE_H_\n";
    r += "#define GENERATED_CODE_H_\n";

    r += "#include \"../../../libraries/c-sdk/include/module-functions.h\"\n\n";

    r += "// Enum and Structure for output GyroAccelMagTemp\n";
    r += "typedef enum\n";
    r += "{\n";
    r += "    accelX =        0b00000000000000000000000000000001,\n";
    r += "    accelY =        0b00000000000000000000000000000010,\n";
    r += "    accelZ =        0b00000000000000000000000000000100,\n";
    r += "    gyroX =         0b00000000000000000000000000001000,\n";
    r += "    gyroY =         0b00000000000000000000000000010000,\n";
    r += "    gyroZ =         0b00000000000000000000000000100000,\n";
    r += "    magX =          0b00000000000000000000000001000000,\n";
    r += "    magY =          0b00000000000000000000000010000000,\n";
    r += "    magZ =          0b00000000000000000000000100000000,\n";
    r += "    temperature =   0b00000000000000000000001000000000\n";
    r += "} fields_GyroAccelMagTemp_t;\n\n";


    r += "typedef struct\n";
    r += "{\n";
    r += "    int accelX;\n";
    r += "    int accelY;\n";
    r += "    int accelZ;\n";
    r += "    int gyroX;\n";
    r += "    int gyroY;\n";
    r += "    int gyroZ;\n";
    r += "    int magX;\n";
    r += "    int magY;\n";
    r += "    int magZ;\n";
    r += "    int temperature;\n";
    r += "} GyroAccelMagTemp_t;\n\n";


    r += "// Enum and Structure for output Baro\n";
    r += "typedef enum\n";
    r += "{\n";
    r += "    pressure =        0b00000000000000000000000000000001\n";
    r += "} fields_Baro_t;\n\n";


    r += "typedef struct\n";
    r += "{\n";
    r += "    int pressure;\n";
    r += "} Baro_t;\n\n";



    r += "typedef struct {\n";
    r += "    module_t module_info;\n";

    r += "    // может не быть если объект без входа\n";
    r += "    GyroAccelMagTemp_t input4modul;\n";

    r += "    shmem_publisher_set_t  GyroAccelMagTemp;\n";
    r += "    shmem_publisher_set_t  Baro;\n";

    r += "    GyroAccelMagTemp_t obj1_GyroAccelMagTemp;\n";
    r += "    GyroAccelMagTemp_t obj2_GyroAccelMagTemp;\n";

    r += "    Baro_t obj1_Baro;\n";
    r += "    Baro_t obj2_Baro;\n";
    r += "} module_GY87_t;\n\n";



    r += "module_GY87_t* c_gy87_create();\n";

    r += "int c_gy87_init(module_GY87_t* module, const uint8_t* bson_data, uint32_t bson_len);\n";

    r += "int c_gy87_start();\n";

    r += "void c_gy87_delete(module_GY87_t* module);\n";

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