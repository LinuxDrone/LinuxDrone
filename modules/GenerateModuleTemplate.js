function GenerateGUID() {
    var d, guid;
    d = new Date().getTime();
    guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx".replace(/[xy]/g, function (c) {
        var r;
        r = (d + Math.random() * 16) % 16 | 0;
        d = Math.floor(d / 16);
        return (c === "x" ? r : r & 0x7 | 0x8).toString(16);
    });
    return guid;
}

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

    r += "include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})\n";
    r += "include_directories(${BSON_INCLUDE_DIR})\n\n";
    r += "include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk)\n";

    r += "set(EXTRA_LIBS\n";
    r += "    sdk\n";
    r += "    pthread\n";
    r += "    rt\n";
    r += ")\n\n";

    r += "ADD_CUSTOM_COMMAND(\n";
    r += "    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/" + module_name + ".helper.c\n";
    r += "    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/" + module_name + ".def.json ${CMAKE_CURRENT_BINARY_DIR} ${PLATFORM}\n";
    r += "    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/" + module_name + ".def.json\n";
    r += ")\n\n";

    r += "add_executable(" + module_name + " ${INC} ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/" + module_name + ".helper.c)\n\n";

    r += "target_link_libraries(" + module_name + " ${EXTRA_LIBS})\n\n";

    r += "IF(${DO_POST_BUILD})\n"
    r += "    add_custom_command(\n";
    r += "        TARGET " + module_name + "\n";
    r += "        POST_BUILD\n";
    r += "        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} " + module_name + " ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/bin/" + module_name + "\n";
    r += "    )\n";
    r += "ENDIF()\n\n";

    r += "install(TARGETS " + module_name + " DESTINATION bin)\n";

    return r;
}

function CreateModuleDefinition(module_name) {
    var obj_def = {
        "type": "module_def",
        "name": module_name,
        "version": 1,
        "rt-priority": 80,
        "main-task-period": 200000,
        "transfer-task-period": 200000,
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
                "type": "const char*",
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
                "type": "bool",
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
        ],
        "inputShema": {
            "type": "object",
            "id": "http://jsonschema.net",
            "properties": {
                "in1": {
                    "type": "float",
                    "required": true,
                    "description": {
                        "en": "input 1",
                        "ru": "Вход 1"
                    }
                },
                "in2": {
                    "type": "float",
                    "required": true,
                    "description": {
                        "en": "input 2",
                        "ru": "Вход 2"
                    }
                }
            }
        },
        "commands": [
            {
                "name": "command0"
            },
            {
                "name": "command1",
                "displayName": {
                    "en": "Command 1",
                    "ru": "Команда 1"
                },
                "description": {
                    "en": "en description",
                    "ru": "русское описание"
                },
                "commandParams": [
                    {
                        "name": "Command Char Param",
                        "displayName": {
                            "en": "param 1",
                            "ru": "параметр 1"
                        },
                        "description": {
                            "en": "en description",
                            "ru": "русское описание"
                        },
                        "type": "const char*",
                        "defaultValue": "default value",
                        "validValues": [
                            "str1",
                            "str2"
                        ],
                        "unitMeasured": ""
                    },
                    {
                        "name": "Command Number Param",
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
                        "name": "Command Boolean Param",
                        "displayName": {
                            "en": "param 3",
                            "ru": "параметр 3"
                        },
                        "description": {
                            "en": "en description",
                            "ru": "русское описание"
                        },
                        "type": "bool",
                        "defaultValue": false,
                        "unitMeasured": "bool"
                    }
                ]
            },
            {
                "name": "command2",
                "description": {
                    "en": "en description",
                    "ru": "русское описание"
                },
                "commandParams": [
                    {
                        "name": "Command float Param",
                        "displayName": {
                            "en": "param 1",
                            "ru": "параметр 1"
                        },
                        "description": {
                            "en": "en description",
                            "ru": "русское описание"
                        },
                        "type": "float",
                        "defaultValue": "default value",
                        "validValues": [
                            "1.0",
                            "2.3"
                        ],
                        "unitMeasured": ""
                    },
                    {
                        "name": "Command Number Param",
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
                        "name": "Command Boolean Param",
                        "displayName": {
                            "en": "param 3",
                            "ru": "параметр 3"
                        },
                        "description": {
                            "en": "en description",
                            "ru": "русское описание"
                        },
                        "type": "bool",
                        "defaultValue": false,
                        "unitMeasured": "bool"
                    }
                ]
            }
        ]
    };

    return obj_def;
}

function CreateModuleCFile(obj_def) {
    var module_name = obj_def.name;
    var r = "";
    r += "#include \"" + module_name + ".helper.h\"\n\n";
    module_name = module_name.replace(/-/g, "_");
    r += "void " + module_name + "_run (module_" + module_name + "_t *module)\n";
    r += "{\n";
    r += "    int cycle=0;\n";
    r += "    while(1) {\n";
    r += "        get_input_data((module_t*)module);\n\n";
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
            var i = 2;
            for (var key in out.Schema.properties) {
                if (out.Schema.properties[key].type == "const char*") {
                    r += "        char buffer_" + key + " [32];\n";
                    r += "        printf(buffer_" + key + ", 32, \"data: %d\", cycle);\n";
                    r += "        obj" + outName + "->" + key + " = buffer_" + key + ";\n";
                } else {
                    if ('inputShema' in obj_def) {
                        r += "        obj" + outName + "->" + key + " = input->in1*" + i + "+cycle;\n";
                        i++;
                    } else {
                        r += "        obj" + outName + "->" + key + " = cycle;\n";
                    }
                }
            }
            r += "//print_" + outName + "(obj" + outName + ");\n";
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
    r += "}\n\n";


    // Функция обработки команды
    r += "void " + module_name + "_command (" + module_name + "_command_t type_command, void* params)\n";
    r += "{\n";
    if ('commands' in obj_def) {
        r += "    switch (type_command)\n";
        r += "    {\n";
        obj_def.commands.forEach(function (cmd) {
            var cmdName = cmd.name.replace(/\-/g, "_");
            r += "        case cmd_" + cmdName + ":\n";
            r += "        break;\n\n";
        });
        r += "        default:\n";
        r += "            printf(\"" + module_name + "_command. Unknown command: %i.\\n\", type_command);\n";
        r += "    }\n";
    }
    r += "}\n";

    return r;
}

function CreateVCProject(obj_def){
    var module_name = obj_def.name;
    var r = "";
    r += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    r += "<Project DefaultTargets=\"Build\" ToolsVersion=\"12.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";
    r += "    <ItemGroup Label=\"ProjectConfigurations\">\n";
    r += "    <ProjectConfiguration Include=\"Debug|Win32\">\n";
    r += "    <Configuration>Debug</Configuration>\n";
    r += "    <Platform>Win32</Platform>\n";
    r += "    </ProjectConfiguration>\n";
    r += "    <ProjectConfiguration Include=\"Debug|x64\">\n";
    r += "    <Configuration>Debug</Configuration>\n";
    r += "    <Platform>x64</Platform>\n";
    r += "    </ProjectConfiguration>\n";
    r += "    <ProjectConfiguration Include=\"Release|Win32\">\n";
    r += "    <Configuration>Release</Configuration>\n";
    r += "    <Platform>Win32</Platform>\n";
    r += "    </ProjectConfiguration>\n";
    r += "    <ProjectConfiguration Include=\"Release|x64\">\n";
    r += "    <Configuration>Release</Configuration>\n";
    r += "    <Platform>x64</Platform>\n";
    r += "    </ProjectConfiguration>\n";
    r += "    </ItemGroup>\n";
    r += "    <PropertyGroup Label=\"Globals\">\n";
    r += "    <ProjectGuid>{" + GenerateGUID() + "}</ProjectGuid>\n";
    r += "    <Keyword>Win32Proj</Keyword>\n";
    r += "    <RootNamespace>" + module_name.replace(/-/g, "") + "</RootNamespace>\n";
    r += "    </PropertyGroup>\n";
    r += "    <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">\n";
    r += "    <ConfigurationType>Application</ConfigurationType>\n";
    r += "    <UseDebugLibraries>true</UseDebugLibraries>\n";
    r += "    <PlatformToolset>v120</PlatformToolset>\n";
    r += "    <CharacterSet>Unicode</CharacterSet>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"Configuration\">\n";
    r += "    <ConfigurationType>Application</ConfigurationType>\n";
    r += "    <UseDebugLibraries>true</UseDebugLibraries>\n";
    r += "    <PlatformToolset>v120</PlatformToolset>\n";
    r += "    <CharacterSet>Unicode</CharacterSet>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\" Label=\"Configuration\">\n";
    r += "    <ConfigurationType>Application</ConfigurationType>\n";
    r += "    <UseDebugLibraries>false</UseDebugLibraries>\n";
    r += "    <PlatformToolset>v120</PlatformToolset>\n";
    r += "    <WholeProgramOptimization>true</WholeProgramOptimization>\n";
    r += "    <CharacterSet>Unicode</CharacterSet>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" Label=\"Configuration\">\n";
    r += "    <ConfigurationType>Application</ConfigurationType>\n";
    r += "    <UseDebugLibraries>false</UseDebugLibraries>\n";
    r += "    <PlatformToolset>v120</PlatformToolset>\n";
    r += "    <WholeProgramOptimization>true</WholeProgramOptimization>\n";
    r += "    <CharacterSet>Unicode</CharacterSet>\n";
    r += "    </PropertyGroup>\n";
    r += "    <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n";
    r += "    <ImportGroup Label=\"ExtensionSettings\">\n";
    r += "    </ImportGroup>\n";
    r += "    <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
    r += "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    r += "    </ImportGroup>\n";
    r += "    <ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\" Label=\"PropertySheets\">\n";
    r += "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    r += "    </ImportGroup>\n";
    r += "    <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">\n";
    r += "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    r += "    </ImportGroup>\n";
    r += "    <ImportGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\" Label=\"PropertySheets\">\n";
    r += "    <Import Project=\"$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')\" Label=\"LocalAppDataPlatform\" />\n";
    r += "    </ImportGroup>\n";
    r += "    <PropertyGroup Label=\"UserMacros\" />\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
    r += "    <LinkIncremental>true</LinkIncremental>\n";
    r += "    <IncludePath>$(SolutionDir)tools\\apr-1.5.1\\include;$(SolutionDir)tools\\libbson-win\\include\\libbson-1.0;$(SolutionDir)libraries\\sdk\\include;$(IncludePath)</IncludePath>\n";
    r += "    <LibraryPath>$(SolutionDir)tools\\apr-1.5.1\x64\\Release;$(LibraryPath)</LibraryPath>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n";
    r += "    <LinkIncremental>true</LinkIncremental>\n";
    r += "    <IncludePath>$(SolutionDir)tools\\apr-1.5.1\\include;$(SolutionDir)tools\\libbson-win\\include\\libbson-1.0;$(SolutionDir)libraries\\sdk\\include;$(IncludePath)</IncludePath>\n";
    r += "    <LibraryPath>$(SolutionDir)tools\\apr-1.5.1\\x64\\Release;$(LibraryPath)</LibraryPath>\n";
    r += "    <PreBuildEventUseInBuild>true</PreBuildEventUseInBuild>\n";
    r += "    <TargetName>$(ProjectName).mod</TargetName>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">\n";
    r += "    <LinkIncremental>false</LinkIncremental>\n";
    r += "    </PropertyGroup>\n";
    r += "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n";
    r += "    <LinkIncremental>false</LinkIncremental>\n";
    r += "    </PropertyGroup>\n";
    r += "    <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
    r += "    <ClCompile>\n";
    r += "    <PrecompiledHeader>\n";
    r += "    </PrecompiledHeader>\n";
    r += "    <WarningLevel>Level3</WarningLevel>\n";
    r += "    <Optimization>Disabled</Optimization>\n";
    r += "    <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;_LIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    r += "    <SDLCheck>true</SDLCheck>\n";
    r += "    </ClCompile>\n";
    r += "    <Link>\n";
    r += "    <SubSystem>Console</SubSystem>\n";
    r += "    <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    r += "    </Link>\n";
    r += "    <PreBuildEvent>\n";
    r += "    <Command>node $(SolutionDir)libraries\\sdk\\ModuleGenerator.js $(ProjectDir)" + module_name + ".def.json $(ProjectDir) MSVC</Command>\n";
    r += "    </PreBuildEvent>\n";
    r += "    </ItemDefinitionGroup>\n";
    r += "    <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">\n";
    r += "    <ClCompile>\n";
    r += "    <PrecompiledHeader>\n";
    r += "    </PrecompiledHeader>\n";
    r += "    <WarningLevel>Level3</WarningLevel>\n";
    r += "    <Optimization>Disabled</Optimization>\n";
    r += "    <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;_LIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    r += "    <SDLCheck>true</SDLCheck>\n";
    r += "    </ClCompile>\n";
    r += "    <Link>\n";
    r += "    <SubSystem>Console</SubSystem>\n";
    r += "    <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    r += "    <AdditionalDependencies>libapr-1.lib;bson-1.0.lib;%(AdditionalDependencies)</AdditionalDependencies>\n";
    r += "    <AdditionalLibraryDirectories>$(SolutionDir)tools\\libbson\\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>\n";
    r += "    </Link>\n";
    r += "    <PreBuildEvent>\n";
    r += "    <Command>node $(SolutionDir)libraries\\sdk\\ModuleGenerator.js $(ProjectDir)" + module_name + ".def.json $(ProjectDir) MSVC</Command>\n";
    r += "    </PreBuildEvent>\n";
    r += "    </ItemDefinitionGroup>\n";
    r += "    <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|Win32'\">\n";
    r += "    <ClCompile>\n";
    r += "    <WarningLevel>Level3</WarningLevel>\n";
    r += "    <PrecompiledHeader>\n";
    r += "    </PrecompiledHeader>\n";
    r += "    <Optimization>MaxSpeed</Optimization>\n";
    r += "    <FunctionLevelLinking>true</FunctionLevelLinking>\n";
    r += "    <IntrinsicFunctions>true</IntrinsicFunctions>\n";
    r += "    <PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;_LIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    r += "    <SDLCheck>true</SDLCheck>\n";
    r += "    </ClCompile>\n";
    r += "    <Link>\n";
    r += "    <SubSystem>Console</SubSystem>\n";
    r += "    <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    r += "    <EnableCOMDATFolding>true</EnableCOMDATFolding>\n";
    r += "    <OptimizeReferences>true</OptimizeReferences>\n";
    r += "    </Link>\n";
    r += "    </ItemDefinitionGroup>\n";
    r += "    <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">\n";
    r += "    <ClCompile>\n";
    r += "    <WarningLevel>Level3</WarningLevel>\n";
    r += "    <PrecompiledHeader>\n";
    r += "    </PrecompiledHeader>\n";
    r += "    <Optimization>MaxSpeed</Optimization>\n";
    r += "    <FunctionLevelLinking>true</FunctionLevelLinking>\n";
    r += "    <IntrinsicFunctions>true</IntrinsicFunctions>\n";
    r += "    <PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;_LIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
    r += "    <SDLCheck>true</SDLCheck>\n";
    r += "    </ClCompile>\n";
    r += "    <Link>\n";
    r += "    <SubSystem>Console</SubSystem>\n";
    r += "    <GenerateDebugInformation>true</GenerateDebugInformation>\n";
    r += "    <EnableCOMDATFolding>true</EnableCOMDATFolding>\n";
    r += "    <OptimizeReferences>true</OptimizeReferences>\n";
    r += "    </Link>\n";
    r += "    </ItemDefinitionGroup>\n";
    r += "    <ItemGroup>\n";
    r += "    <ClCompile Include=\"" + module_name + ".c\" />\n";
    r += "    <ClCompile Include=\"" + module_name + ".helper.c\" />\n";
    r += "    </ItemGroup>\n";
    r += "    <ItemGroup>\n";
    r += "    <ClInclude Include=\"" + module_name + ".helper.h\" />\n";
    r += "    </ItemGroup>\n";
    r += "    <ItemGroup>\n";
    r += "    <ProjectReference Include=\"..\\..\\libraries\\sdk\\sdk.vcxproj\">\n";
    r += "    <Project>{c239d93d-f3d7-49f8-b198-4d9965f96af9}</Project>\n";
    r += "    </ProjectReference>\n";
    r += "    </ItemGroup>\n";
    r += "    <Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />\n";
    r += "    <ImportGroup Label=\"ExtensionTargets\">\n";
    r += "    </ImportGroup>\n";
    r += "    </Project>\n";

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


    fs.writeFile(module_name + "/" + module_name + '.vcxproj', CreateVCProject(obj_def), function (err) {
        if (err) return console.log(err);
    });
}

main();