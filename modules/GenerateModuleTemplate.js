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

    r += "file(GLOB_RECURSE INC \"*.h\")\n";
    r += "file(GLOB_RECURSE SRC \"*.c\")\n\n";

    r += "include_directories(${LIB_DIR}/c-sdk/include ${CMAKE_CURRENT_BINARY_DIR})\n";
    r += "include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)\n\n";

    r += "set(EXTRA_LIBS\n";
    r += "    c-sdk\n";
    r += ")\n\n";

    r += "ADD_CUSTOM_COMMAND(\n";
    r += "    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/" + module_name + ".helper.c\n";
    r += "    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/" + module_name + ".def.json ${CMAKE_CURRENT_BINARY_DIR}\n";
    r += "    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/" + module_name + ".def.json\n";
    r += ")\n\n";

    r += "add_library(" + module_name + " MODULE ${INC} ${SRC} " + module_name + ".helper.c)\n\n";

    r += "target_link_libraries(" + module_name + " ${EXTRA_LIBS})\n\n";

    r += "IF(${DO_POST_BUILD})\n"
    r += "    add_custom_command(\n";
    r += "        TARGET " + module_name + "\n";
    r += "        POST_BUILD\n";
    r += "        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} lib" + module_name + ".so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/" + module_name + "/lib" + module_name + ".so\n";
    r += "        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/" + module_name + "/" + module_name + ".def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/" + module_name + "/" + module_name + ".def.json\n";
    r += "    )\n";
    r += "ENDIF()\n\n";

    r += "install(TARGETS " + module_name + " DESTINATION modules/" + module_name + ")\n";
    r += "install(FILES " + module_name + ".def.json DESTINATION modules/" + module_name + ")\n";

    return r;
}

function CreateModuleDefinition(module_name) {
    var obj_def = {
  "type": "module_def",
  "name": module_name,
  "version": 2,
  "Task Priority": 80,
  "Task Period": 200000,
  "Transfer task period": 200000,
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
      "validValues": [
        "str1",
        "str2"
      ],
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
      "type": "int",
      "defaultValue": 123,
      "validValues": [
        56,
        123
      ],
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
          "char_out": {
            "type": "char",
            "required": true
          },
          "short_out": {
            "type": "short",
            "required": true
          },
          "int_out": {
            "type": "int",
            "required": true
          },
          "long_out": {
            "type": "long",
            "required": true
          },
          "long_long_out": {
            "type": "long long",
            "required": true
          },
          "float_out": {
            "type": "float",
            "required": true
          },
          "double_out": {
            "type": "double",
            "required": true
          },
          "string_out": {
            "type": "const char*",
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
          "out3": {
            "type": "int",
            "required": true
          }
        }
      }
    }
  ]
};

    return obj_def;
}

function CreateModuleCFile(obj_def){
    var module_name = obj_def.name;
    var r = "";
    r += "#include \"" + module_name + ".helper.h\"\n\n";
    module_name = module_name.replace(/-/g, "_");
    r += "void "+module_name+"_run (module_"+module_name+"_t *module)\n";
    r += "{\n";
    r += "    int cycle=0;\n";
    r += "    while(1) {\n";
    r += "        get_input_data(module);\n\n";
    r += "        // проверим, обновились ли данные\n";
    r += "        if(module->module_info.updated_input_properties!=0)\n";
    r += "        {\n";
    r += "            // есть новые данные\n";
    r += "        }\n";
    r += "        else\n";
    r += "        {\n";
    r += "            // вышел таймаут\n";
    r += "        }\n\n";

    if ('inputShema' in obj_def) {
        r += "        input_t* input = (input_t*)module->module_info.input_data;\n\n";
    }

    if ('outputs' in obj_def) {
        obj_def.outputs.forEach(function (out) {
            var outName = out.name.replace(/\+/g, "");
            r += "        " + outName + "_t* obj" + outName + ";\n";
            r += "        checkout_" + outName + "(module, &obj" + outName + ");\n";
            var i=2;
            for (var key in out.Schema.properties) {
                if(out.Schema.properties[key].type=="const char*"){
                    r += "        char buffer_" + key + " [32];\n";
                    r += "        itoa (cycle, buffer_" + key+ ", 10);\n";
                    r += "        obj" + outName + "->" + key + " = buffer_" + key + ";\n";
                }else{
                    if ('inputShema' in obj_def) {
                        r += "        obj" + outName + "->" + key + " = input->in1*"+i+"+cycle;\n";
                        i++;
                    }else{
                        r += "        obj" + outName + "->" + key + " = cycle;\n";
                    }
                }
            }
            r += "        checkin_" + outName + "(module, &obj" + outName + ");\n\n";
        });
    }

    if ('inputShema' in obj_def) {
        var mask = "";
        for (var key in obj_def.inputShema.properties) {
            if (mask != "") {
                mask += " | ";
            }
            mask += key;
        }

        r += "        // Скажем какие данные следует добыть из разделяемой памяти, если они не придут через трубу\n";
        r += "        module->module_info.refresh_input_mask = " + mask + ";\n\n";
        r += "        // Принудительное считывание данных из разделяемой памяти\n";
        r += "        //int res = refresh_input(module);\n\n";
    }
    r += "        cycle++;\n";
    r += "    }\n";
    r += "}\n";

    return r;
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


    var obj_def = CreateModuleDefinition(module_name);
    fs.writeFile(module_name + "/" + module_name + '.def.json', JSON.stringify(obj_def, undefined, 2), function (err) {
        if (err) return console.log(err);
    });


    fs.writeFile(module_name + "/" + module_name + '.c', CreateModuleCFile(obj_def), function (err) {
        if (err) return console.log(err);
    });

    //process.argv[2] = module_name + "/" + module_name + '.def.json';
    //process.argv[3] = module_name;
    //require('../libraries/c-sdk/ModuleGenerator.js');
}


main();