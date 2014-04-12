function CreateCMakeLists(module_name) {
    var r = "";
    r += "#--------------------------------------------------------------------\n";
    r += "# This file was created as a part of the LinuxDrone project:\n";
    r += "#                http://www.linuxdrone.org\n";
    r += "#\n";
    r += "# Distributed under the Creative Commons Attribution-ShareAlike 4.0\n";
    r += "# International License (see accompanying License.txt file or a copy\n";
    r += "# at http://creativecommons.org/licenses/by-sa/4.0/legalcode)\n";
    r += "#\n";
    r += "# The human-readable summary of (and not a substitute for) the\n";
    r += "# license: http://creativecommons.org/licenses/by-sa/4.0/\n";
    r += "#--------------------------------------------------------------------\n\n";

    r += "file(GLOB_RECURSE INC \"include/*\")\n";
    r += "file(GLOB_RECURSE SRC \"src/*.c\")\n\n";

    r += "include_directories(${LIB_DIR}/c-sdk/include)\n";
    r += "include_directories(../../tools/rootfs/beaglebone/usr/local/include/libbson-1.0)\n\n";

    r += "set(EXTRA_LIBS\n";
    r += "    c-sdk\n";
    r += ")\n\n";

    r += "add_library(" + module_name + " MODULE ${INC} ${SRC})\n\n";

    r += "target_link_libraries(" + module_name + " ${EXTRA_LIBS})\n\n";

    r += "add_custom_command(\n";
    r += "    TARGET " + module_name + "\n";
    r += "    PRE_BUILD\n";
    r += "    COMMAND /opt/local/bin/node ${LIB_DIR}/c-sdk/ModuleGenerator.js ${MOD_DIR}/" + module_name + "/" + module_name + ".def.json ${MOD_DIR}/" + module_name + "\n";
    r += ")\n\n";

    r += "install(TARGETS " + module_name + " DESTINATION modules/" + module_name + ")\n";
    r += "install(FILES " + module_name + ".def.json DESTINATION modules/" + module_name + ")\n";

    return r;
}

function CreateModuleDefinition(module_name) {
    var obj_def = {
        "type": "module_def",
        "name": module_name,
        "version": 1,
        "Task Priority": 80,
        "Task Period": 200,
        "Notify on change": false,
        "description": {
            "en": "en description",
            "ru": "русское описание"
        },
        "paramsDefinitions": [
            {
                "name": "I2C Device",
                "displayName": {
                    "en": "param 1",
                    "ru": "параметр 1"
                },
                "description": {
                    "en": "en description",
                    "ru": "русское описание"
                },
                "type": "string",
                "defaultValue": "default value",
                "validValues": ["str1", "str2"],
                "unitMeasured": ""
            },
            {
                "name": "Test Number Param",
                "displayName": {
                    "en": "param 2",
                    "ru": "параметр 2"
                },
                "description": {
                    "en": "en description",
                    "ru": "русское описание"
                },
                "type": "number",
                "defaultValue": 123,
                "validValues": [56, 123],
                "unitMeasured": "Ms"
            },
            {
                "name": "Test Boolean Param",
                "displayName": {
                    "en": "param 3",
                    "ru": "параметр 3"
                },
                "description": {
                    "en": "en description",
                    "ru": "русское описание"
                },
                "type": "boolean",
                "defaultValue": false,
                "unitMeasured": "bool"
            }
        ],
        "outputs": [
            {
                "name": "Output1",
                "Schema": {
                    "type": "object",
                    "id": "http://jsonschema.net",
                    "properties": {
                        "out1": {
                            "type": "number",
                            "required": true
                        },
                        "out2": {
                            "type": "number",
                            "required": true
                        }
                    }
                }
            },
            {
                "name": "Output2",
                "Schema": {
                    "type": "object",
                    "id": "http://jsonschema.net",
                    "properties": {
                        "pressure": {
                            "type": "number",
                            "required": true
                        }
                    }
                }
            }
        ],
        "inputShema": {
            "type": "object",
            "id": "http://jsonschema.net",
            "properties": {
                "pwm0": {
                    "type": "number",
                    "required": true,
                    "description": {
                        "en": "input 2",
                        "ru": "Вход 1"
                    }
                },
                "pwm1": {
                    "type": "number",
                    "required": true,
                    "description": {
                        "en": "input 2",
                        "ru": "Вход 2"
                    }
                }
            }
        }
    };

    return JSON.stringify(obj_def, undefined, 2);
}

function main() {
    if (process.argv.length < 3) {
        console.log("Use " + process.argv[0] + " " + process.argv[1] + " YOUR_MODULE_NAME");
        return -1;
    }

    var module_name = process.argv[2];
    var fs = require('fs');

    if (!fs.existsSync(module_name)) {
        fs.mkdirSync(module_name, 0766, function (err) {
            if (err) {
                console.log(err);
                response.send("ERROR! Can't make the directory! `\n");    // echo the result back
            }
        });
    }

    var CMakeLists_file = CreateCMakeLists(module_name);
    //console.log(text_H_file);
    fs.writeFile(module_name + '/CMakeLists.txt', CMakeLists_file, function (err) {
        if (err) return console.log(err);
    });


    fs.writeFile(module_name + "/" + module_name + '.def.json', CreateModuleDefinition(module_name), function (err) {
        if (err) return console.log(err);
    });

    var src_folder = module_name + "/src";
    if (!fs.existsSync(src_folder)) {
        fs.mkdirSync(src_folder, 0766, function (err) {
            if (err) {
                console.log(err);
                response.send("ERROR! Can't make the directory! `\n");    // echo the result back
            }
        });
    }

    var include_folder = module_name + "/include";
    if (!fs.existsSync(include_folder)) {
        fs.mkdirSync(include_folder, 0766, function (err) {
            if (err) {
                console.log(err);
                response.send("ERROR! Can't make the directory! `\n");    // echo the result back
            }
        });
    }

    process.argv[2] = module_name + "/" + module_name + '.def.json';
    process.argv[3] = module_name;
    require('../libraries/c-sdk/ModuleGenerator.js');
}


main();