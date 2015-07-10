var graph = new joint.dia.Graph;
var BSON = bson().BSON;

var BrowserDetect = {
    init: function () {
        this.browser = this.searchString(this.dataBrowser) || "An unknown browser";
        this.version = this.searchVersion(navigator.userAgent)
            || this.searchVersion(navigator.appVersion)
            || "an unknown version";
        this.OS = this.searchString(this.dataOS) || "an unknown OS";
    },
    searchString: function (data) {
        for (var i=0;i<data.length;i++)	{
            var dataString = data[i].string;
            var dataProp = data[i].prop;
            this.versionSearchString = data[i].versionSearch || data[i].identity;
            if (dataString) {
                if (dataString.indexOf(data[i].subString) != -1)
                    return data[i].identity;
            }
            else if (dataProp)
                return data[i].identity;
        }
    },
    searchVersion: function (dataString) {
        var index = dataString.indexOf(this.versionSearchString);
        if (index == -1) return;
        return parseFloat(dataString.substring(index+this.versionSearchString.length+1));
    },
    dataBrowser: [
        {
            string: navigator.userAgent,
            subString: "Chrome",
            identity: "Chrome"
        },
        { 	string: navigator.userAgent,
            subString: "OmniWeb",
            versionSearch: "OmniWeb/",
            identity: "OmniWeb"
        },
        {
            string: navigator.vendor,
            subString: "Apple",
            identity: "Safari",
            versionSearch: "Version"
        },
        {
            prop: window.opera,
            identity: "Opera",
            versionSearch: "Version"
        },
        {
            string: navigator.vendor,
            subString: "iCab",
            identity: "iCab"
        },
        {
            string: navigator.vendor,
            subString: "KDE",
            identity: "Konqueror"
        },
        {
            string: navigator.userAgent,
            subString: "Firefox",
            identity: "Firefox"
        },
        {
            string: navigator.vendor,
            subString: "Camino",
            identity: "Camino"
        },
        {		// for newer Netscapes (6+)
            string: navigator.userAgent,
            subString: "Netscape",
            identity: "Netscape"
        },
        {
            string: navigator.userAgent,
            subString: "MSIE",
            identity: "Explorer",
            versionSearch: "MSIE"
        },
        {
            string: navigator.userAgent,
            subString: "Gecko",
            identity: "Mozilla",
            versionSearch: "rv"
        },
        { 		// for older Netscapes (4-)
            string: navigator.userAgent,
            subString: "Mozilla",
            identity: "Netscape",
            versionSearch: "Mozilla"
        }
    ],
    dataOS : [
        {
            string: navigator.platform,
            subString: "Win",
            identity: "Windows"
        },
        {
            string: navigator.platform,
            subString: "Mac",
            identity: "Mac"
        },
        {
            string: navigator.userAgent,
            subString: "iPhone",
            identity: "iPhone/iPod"
        },
        {
            string: navigator.platform,
            subString: "Linux",
            identity: "Linux"
        }
    ]

};
BrowserDetect.init();

var paper = new joint.dia.Paper({
    el: $('#paper'),
    gridSize: 20,
    model: graph,
    defaultLink: new joint.dia.Link({
        attrs: {
            '.marker-target': {fill: 'red', d: 'M 10 0 L 0 5 L 10 10 z' },
            '.connection': {stroke: 'red', 'stroke-width': "2"}
        }
    }),
    validateConnection: function (cellViewS, magnetS, cellViewT, magnetT) {
        //return true;
        // Prevent linking from input ports.
        if (magnetS && magnetS.attributes.fill.value === viewModels.Editor.inPortsFillColor) return false;
        // Prevent linking from output ports to input ports within one element.
        if (cellViewS === cellViewT) return false;
        // Prevent linking to input ports.
        return magnetT && magnetT.attributes.fill.value === viewModels.Editor.inPortsFillColor;
    }
    /*
     ,
     validateMagnet: function (cellView, magnet) {
     // Note that this is the default behaviour. Just showing it here for reference.
     // Disable linking interaction for magnets marked as passive (see below `.inPorts circle`).
     return magnet.getAttribute('magnet') !== 'passive';
     }
     */
});
$(paper.el).find('svg').attr('preserveAspectRatio', 'xMinYMin');
$(paper.el).find('svg').css({
    width: "100%",
    height: "99%"
});

var viewModels = viewModels || {};

viewModels.Editor = (function () {
    var outPortsFillColor = '#E74C3C';
    var normalModuleColor = '#2ECC71';
    var smartModuleColor = '#CCCC71';
    var blockColor = '#CC4C71';

    var allConfigs = {};

    var res = {
        inPortsFillColor: '#16A085',
        // Режим отображения схем. Два варианта - "main" для рисования схем верхнего уровня, и "sub" для
        // рисования подсхем конфигурации сложных модулей
        editSchemaMode: ko.observable("main"),
        // Метаинформация модулей
        metaModules: ko.observableArray([]),
        // Список модулей
        listModules: ko.observableArray([]),
        // Список имен конфигураций
        ConfigNames: ko.observableArray([]),
        // Выбранное имя конфигурации
        configNameSelected: ko.observable(),
        // Список версий для выбранного имени конфигурации
        Versions: ko.observableArray([]),
        // Выбранная версия
        versionSelected: ko.observable(),
        // Новое название конфигурации (связано с полем в диалоге "Сохранить как")
        newConfigName: ko.observable(),
        // Новая версия конфигурации (связано с полем в диалоге "Сохранить как")
        newConfigVersion: ko.observable(),
        // Принимает true, когда в схему внесено изменение
        graphChanged: ko.observable(),
        // Текущая редактируемая конфигурация
        currentConfig: ko.observable(),
        // Модуль выбранный на схеме
        selectedCell: ko.observable(),
        // Связь выбранная на схеме
        selectedLink: ko.observable(),
        // Имя инстанса выбранного в схеме модуля
        instnameSelectedModule: ko.observable(),
        // служит для хранения ссылки на model сложноконфигурируемого модуля при переходе к редактированию его схемы
        selectedSuperModule: ko.observable(),

        // Общие свойства выбранного в схеме инстанса модуля
        instanceCommonProperties: ko.observableArray([]),
        // Специфические свойства выбранного в схеме инстанса модуля
        instanceSpecificProperties: ko.observableArray([]),

        // Статус хост программы
        // running, stopped
        hostStatus:ko.observable(),

        cssClass4ButtonsRunStop:ko.observable(),

        // Загрузка процессора задачами ксеномая
        XenoCPU:ko.observable()
    };


    // Публичная функция загрузки конфигурации
    res.LoadConfigurations = function (_initialData) {
        allConfigs = _initialData;

        if (allConfigs.length == 0) {
            allConfigs.push({
                    "name": "New Schema",
                    "version": 1
                }
            );
        }

        res.ConfigNames([]);
        $.each(_.groupBy(allConfigs, 'name'), function (configName) {
            res.ConfigNames.push(configName);
        });
    };

    var MetaOfModule = function () {
        var self = this;
        self.definition = ko.observable();
        self.AddModule2Paper = function () {
            graph.addCell(MakeVisualModule(self));
        };
        self.GetModuleDescription = function () {

            return Geti18nProperty(this.definition(), "description");

            //return "In the future there will be a description of the module";
        };
    };

    // Публичная функция загрузки метаинформации модулей
    res.LoadMetaModules = function (_initialData) {
        res.metaModules([]);
        $.each(_initialData, function (i, item) {
            var el = new MetaOfModule()
                .definition(item);
            res.metaModules.push(el);
            res.listModules.push(el);
        });
    };

    // Публичная функция сохранения текущей конфигурации
    // Название и версия берутся из комбобоксов имени и версий
    res.SaveConfig = function SaveConfig() {
        SaveCurrentConfig(res.configNameSelected(), res.versionSelected());
    }

    // Публичная функция сохранения текущей конфигурации под новым именем
    // Название и версия берутся из полей ввода диалога "Сохранить как.."
    res.SaveConfigAs = function SaveConfigAs() {
        SaveCurrentConfig(res.newConfigName(), res.newConfigVersion(), true);
    }

    // Публичная функция удаления текущей конфигурации
    res.DeleteConfig = function DeleteConfig() {
        var data4Send = {
            "name": res.configNameSelected(),
            "version": res.versionSelected()
        };
        $.post("delconfig", data4Send,
            function (data) {
                if (data == "OK") {
                    allConfigs = _.reject(allConfigs, function (cfg) {
                        return cfg.name == data4Send.name && cfg.version == data4Send.version;
                    });

                    // Если в списке версий осталась одна версия, то просто удалим название конфигурации из списка
                    // конфигураций. Иначе удалим версию из списка версий
                    if (res.Versions().length == 1) {
                        res.ConfigNames.splice(res.ConfigNames.indexOf(res.configNameSelected()), 1);
                    }
                    else {
                        res.Versions.splice(res.Versions.indexOf(res.versionSelected()), 1);
                    }
                }
            });
    }

    // Публичная функция запуска текущей конфигурации
    res.RunConfig = function() {
        //paper.scale(0.5, 0.5);
        var data4send = {
            "name": res.configNameSelected(),
            "version": res.versionSelected()
        };
        $.post("runhosts", data4send,
            function (data) {
                Subscribe2Telemetry("subscribe");
            });
    }

    // Приватная. Подписаться, отписаться на телеметрию всех инстансов
    var Subscribe2Telemetry = function(cmd){
        if(socketTelemetry.readyState==1) {
            _.find(graph.getElements(), function (el) {
                var moduleType = el.attributes.moduleType;
                var outputs = GetModuleMeta(moduleType).definition().outputs;
                if (outputs) {
                    outputs.forEach(function (output) {
                        var obj = {
                            cmd: cmd,
                            instance: el.attributes.attrs['.label'].text,
                            out: output.name
                        };
                        var data = BSON.serialize(obj, true, true, false);
                        socketTelemetry.send(data.buffer);
                    });
                }
            });
        }
    }


    // Публичная функция остановки текущей конфигурации
    res.StopConfig = function() {
        Subscribe2Telemetry("unsubscribe");

        var data4send = {
            "name": res.configNameSelected(),
            "version": res.versionSelected()
        };
        $.post("stophosts", data4send,
            function (data) {
                var f=0;
            });
    }


    res.RemoveModule = function RemoveModule() {
        if (res.selectedCell()) {
            // Следует так же удалить настройки модуля
            // Если это блок, то и удалять будем хитрее
            if (res.selectedSuperModule()) {
                var parentSuperModuleName = res.selectedSuperModule().attributes.attrs[".label"].text;
                delete res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig[res.selectedCell().attributes.attrs['.label'].text];
            }
            else {
                delete res.currentConfig().modulesParams[res.selectedCell().attributes.attrs['.label'].text];
            }

            res.selectedCell().remove();
            res.instnameSelectedModule("Properties");
            $("#moduleContextMenu").hide();
            res.graphChanged(true);
        }
    }

    res.SetLinkAsPipe = function SetLinkAsPipe() {
        var selectedLink = res.selectedLink().model;
        if (selectedLink) {
            selectedLink.attributes["mode"] = "queue";
            res.selectedLink().model.attributes.attrs[".connection"] = { stroke: 'red' };
            res.selectedLink().model.attributes.attrs[".marker-target"].fill = 'red';
            res.selectedLink().update();

            $("#linkContextMenu").hide();
            res.graphChanged(true);
        }
    }

    res.SetLinkAsSharedMemory = function SetLinkAsSharedMemory() {
        var selectedLink = res.selectedLink().model;
        if (selectedLink) {
            selectedLink.attributes["mode"] = "memory";
            res.selectedLink().model.attributes.attrs[".connection"] = { stroke: 'blue' };
            res.selectedLink().model.attributes.attrs[".marker-target"].fill = 'blue';
            res.selectedLink().update();

            $("#linkContextMenu").hide();
            res.graphChanged(true);
        }
    }

    res.chooseTemplate4Property = function chooseTemplate4Property(metaProperty) {
        if (metaProperty.type == "boolean") {
            return "boolTemplate";
        }
        else {
            return "stringTemplate";
        }
    };

    // Возврат к редактированию основной схемы, из экрана конфигурирования сложного модуля
    res.ReturnToMainScheme = function () {
        res.listModules.removeAll();
        res.editSchemaMode("main");
        res.instnameSelectedModule("Properties");

        $.each(res.metaModules(), function (i, el) {
            res.listModules.push(el);
        });
        ShowMainSchema(res.versionSelected());

        var modelSuperModule = _.find(graph.attributes.cells.models, function (model) {
            return model.id == res.selectedSuperModule().id;
        });

        if (res.selectedSuperModule().attributes.blocksJSON.cells) {
            modelSuperModule.attributes.blocksJSON = res.selectedSuperModule().attributes.blocksJSON;
            res.currentConfig().jsonGraph = JSON.stringify(graph.toJSON());
            res.graphChanged(true);
        }
        res.selectedSuperModule(undefined);
    }

    res.SaveSubSchema = function () {
        res.selectedSuperModule().attributes.blocksJSON = graph.toJSON();
    }

    // Пытается вернуть текстовое свойство объекта в локали браузера
    var Geti18nProperty = function (obj, propName) {
        if (obj.hasOwnProperty(propName)) {
            if (obj[propName].hasOwnProperty(GetLocale())) {
                return obj[propName][GetLocale()];
            }
            else {
                if (obj[propName].hasOwnProperty("en")) {
                    return obj.description.en;
                }
            }
        }
        return "";
    };

    // Приватная функция сохранения конфигурации (текущего графа) с именем и версией
    var SaveCurrentConfig = function (name, version, isNew) {
        var data4save = {
            "name": name,
            "version": version,
            "jsonGraph": JSON.stringify(graph.toJSON()),
            "modulesParams": res.currentConfig().modulesParams
        };
        $.post("saveconfig", data4save,
            function (data) {
                if (data != "OK") {
                    alert(data);
                }
                else {
                    if (isNew) {
                        allConfigs.push(data4save);
                        // Если была записана новая конфигурация, добавим ее в комбобоксы
                        // Если была записана новая версия, а имя конфигурации не изменилось, то добавим новую строку только
                        // в комбобокс версий.
                        // Иначе добавим новые строки в оба комбобокса. И установим как выбранные значения в комбобоксах,
                        // соответсвующие имени и версии новой конфигурации
                        if (_.contains(res.ConfigNames(), name)) {
                            if (res.configNameSelected() == name) {
                                // Добавим контент конфигурации к списку версий выбранной конфигурации
                                res.Versions.push(version);
                            }
                            else {
                                res.configNameSelected(name);
                            }
                            res.versionSelected(version);
                        }
                        else {
                            res.ConfigNames.push(name);
                            res.configNameSelected(name);
                        }
                    }
                    else {
                        allConfigs = _.reject(allConfigs, function (cfg) {
                            return cfg.name == name && cfg.version == version;
                        });
                        allConfigs.push(data4save);
                    }
                    res.graphChanged(false);
                }
            });
    };

    // Приватная функция
    // Возвращает объект текущего редактируемого конфига (из списка всех конфигов allConfigs)
    var GetConfig = function (configName, version) {
        var cfg = _.where(allConfigs, {name: configName, version: version})[0];
        cfg.modulesParams = cfg.modulesParams || {};
        //cfg.linksParams = cfg.linksParams || {};
        return cfg;
    };

    // Приватная функция
    // Возвращает объект - значения общих конфигурационных параметров инстанса модуля
    var GetInstanceCommonParams = function GetInstanceCommonParams(instanceName, moduleMeta) {
        return GetInstanceParams(instanceName, moduleMeta).common;
    };

    // Приватная функция
    // Возвращает объект - значения общих конфигурационных параметров инстанса модуля
    var GetInstanceSpecificParams = function GetInstanceSpecificParams(instanceName, moduleMeta) {
        return GetInstanceParams(instanceName, moduleMeta).specific;
    };

    // Приватная функция
    // Возвращает объект - значения конфигурационных параметров инстанса модуля
    var GetInstanceParams = function GetInstanceParams(instanceName, moduleMeta) {
        var moduleParams;
        var requireMakeDefaults = false;
        switch (moduleMeta.definition().type) {
            case "module_def":
                if (!res.currentConfig().modulesParams[instanceName]) {
                    requireMakeDefaults = true;
                    res.currentConfig().modulesParams[instanceName] = {common: {}, specific: {}};
                }
                moduleParams = res.currentConfig().modulesParams[instanceName];
                break;

            case "block_def":
                // Если это не модуль а блок, поместим его параметры в иерархию соответсвующего родительского инстанса модуля
                var parentSuperModuleName = res.selectedSuperModule().attributes.attrs[".label"].text;
                if (!res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig) {
                    res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig = {};
                }

                if (!res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig[instanceName]) {
                    requireMakeDefaults = true;
                    res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig[instanceName] = {specific: {}};
                }
                moduleParams = res.currentConfig().modulesParams[parentSuperModuleName].blocksConfig[instanceName];
                break;

            default:
                alert("Unknown module object type '" + moduleMeta.definition().type + "'");
        }

        if (requireMakeDefaults) {
            // Если для указанного инстанса нет в конфигурации параметров, следует их создать на основе дефолтных
            // из метаописания модуля
            if (moduleMeta.definition().type == "module_def") {
                // Сначала заполним дефолтные значения для общих (для всех типов модулей) свойств
                $.each(_.pluck(ModulesCommonParams.commonModuleParamsDefinition, "name"), function (i, paramName) {
                    // Если дефолтное значение задано в определении модуля, то используем его
                    // Иначе (если опять же оно задано) возьмем его из общего для всех модулей определения
                    if (moduleMeta.definition()[paramName]) {
                        moduleParams.common[paramName] = moduleMeta.definition()[paramName];
                    }
                    else {
                        var commonParam = _.find(ModulesCommonParams.commonModuleParamsDefinition, function (commonParamMeta) {
                            return commonParamMeta.name == paramName;
                        });
                        if (commonParam && commonParam.defaultValue) {
                            moduleParams.common[paramName] = commonParam.defaultValue;
                        }
                    }
                });
            }

            // Теперь установим специфичные для модуля параметры, взяв их значения из определения типа модуля
            if (moduleMeta.definition().paramsDefinitions) {
                $.each(moduleMeta.definition().paramsDefinitions, function (i, paramDefinition) {
                    moduleParams.specific[paramDefinition.name] = paramDefinition.defaultValue;
                });
            }
        }
        return moduleParams;
    };

    // Приватная функция
    // Возвращает описание модуля
    var GetModuleMeta = function GetModuleMeta(moduleName) {
        var result = _.find(res.metaModules(), function (meta) {
            return meta.definition().name == moduleName;
        });
        if (result) {
            return result;
        }

        result = _.find(res.listModules(), function (meta) {
            return meta.definition().name == moduleName;
        });

        if (!result) {
            alert("Not found module type " + moduleName);
        }

        return result;
    };

    // Приватная функция
    // Создает экзмепляр визуального модуля библиотеки joint, из описания модуля в формате linuxdrone
    var MakeVisualModule = function MakeVisualModule(moduleInfo) {
        var moduleDef = moduleInfo.definition();

        var maxPins = 0;

        var module = {
            moduleType: moduleDef.name,
            position: { x: 10, y: 20 },
            size: { width: 90},
            attrs: {
                '.label': {'ref-x': .2, 'ref-y': -2 },
                rect: {fill: normalModuleColor},
                '.inPorts circle': { fill: res.inPortsFillColor },
                '.outPorts circle': { fill: outPortsFillColor }
            }
        };


        switch (moduleDef.type) {
            case "module_def":
                if (moduleDef.subSchema) {
                    // Если это модуль сложноконфигурируемый и содержащий собственную подсхему
                    module.attrs.rect.fill = smartModuleColor;
                    var graphBlocks = new joint.dia.Graph;
                    module.blocksJSON = graphBlocks.toJSON();
                }
                break;

            case "block_def":
                module.attrs.rect.fill = blockColor;
                break;
        }


        var instancesCount = 1;
        var name4NewInstance = moduleDef.name + "-" + instancesCount;
        // Проверим не используется ли данное имя уже в качестве имени инстанса а схеме
        while (_.find(graph.getElements(), function (el) {
            return el.attributes.attrs['.label'].text == name4NewInstance;
        })) {
            instancesCount++;
            name4NewInstance = moduleDef.name + "-" + instancesCount;
        }

        module.attrs['.label'].text = name4NewInstance;

        if (moduleDef.outputs) {
            module.outPorts = new Array();
            $.each(moduleDef.outputs, function (i, output) {
                var propsCount = Object.keys(output.Schema.properties).length;
                maxPins += propsCount;
                $.each(output.Schema.properties, function (i, pin) {
                    module.outPorts.push(i);
                });
            });
        }

        if (moduleDef.inputShema) {
            module.inPorts = new Array();
            var propsCount = Object.keys(moduleDef.inputShema.properties).length;
            if (maxPins < propsCount) {
                maxPins = propsCount;
            }
            $.each(moduleDef.inputShema.properties, function (i, pin) {
                module.inPorts.push(i);
            });
        }

        module.size.height = maxPins * 30;

        // Этот вывзов обеспечит установку значений по умолчанию для свойств
        GetInstanceParams(name4NewInstance, moduleInfo);

        // Добавление параметров, со значениями по умолчанию
        return new joint.shapes.devs.Model(module);
    };

    var GetLocale = function () {
        var l_lang;
        if (navigator.userLanguage) // Explorer
            l_lang = navigator.userLanguage;
        else if (navigator.language) // FF
            l_lang = navigator.language;
        else
            l_lang = "en";
        return l_lang;
    };

    var ShowMainSchema = function (version) {
        if (version) {
            res.currentConfig(GetConfig(res.configNameSelected(), version));
            if (res.currentConfig().jsonGraph) {
                graph.fromJSON(JSON.parse(res.currentConfig().jsonGraph));

                // Инициализация портов на модулях, с целью показа их описания в тултипе
                var elements = graph.getElements();
                $.each(elements, function (i, element) {
                    var view = paper.findViewByModel(element);

                    $.each([2, 3], function (i, s) {
                        $.each(view.el.childNodes[0].childNodes[s].childNodes, function (k, port) {
                            port.childNodes[0].onmouseenter = function (e) {
                                $("#portTooltip").css({
                                    display: "block",
                                    left: e.x,
                                    top: e.y
                                });
                                var groupsPorts = new Array();
                                var moduleType = element.attributes.moduleType;
                                var portName = port.textContent;

                                var meta = GetModuleMeta(moduleType);
                                if (s == 2) {
                                    groupsPorts.push(meta.definition().inputShema.properties);
                                }
                                else {
                                    $.each(meta.definition().outputs, function (i, m) {
                                        groupsPorts.push(m.Schema.properties);
                                    });
                                }

                                $.each(groupsPorts, function (i, gp) {
                                    var propsObj = gp;
                                    if (propsObj.hasOwnProperty(portName)) {
                                        var portMeta = propsObj[portName];

                                        var text = portMeta.type;

                                        text += "<br>" + Geti18nProperty(portMeta, "description");

                                        var options = {placement: "left", html: true, title: text};
                                        $("#portTooltip").tooltip('destroy');
                                        $("#portTooltip").tooltip(options);
                                        $("#portTooltip").tooltip('show');
                                    }
                                });
                            };

                            port.childNodes[0].onmouseleave = function (e) {
                                $("#portTooltip").tooltip('hide');
                            }
                        });
                    });
                });
            }
            res.graphChanged(false);
            res.instnameSelectedModule("Properties");

            PrepareListLinks();
        }
    }

    // Приватная
    // Возвращает имя типа модйля по идентификатору модуля в графической схеме
    var GetModuleTypeByGraphId = function(IdModelInGraph)
    {
        return _.find(graph.attributes.cells.models, function (model) {
            return model.id == IdModelInGraph;
        }).attributes.moduleType;
    }

    // Загрузка метаданных общих свойств модулей в observable переменную
    ModulesCommonParams.commonModuleParamsDefinition.forEach(function (prop) {
        prop.value = ko.observable(prop.defaultValue);
        res.instanceCommonProperties.push(prop);
    });

    // Обработчик события выбора имени конфигурации
    res.configNameSelected.subscribe(function (selectedName) {
        res.Versions([]);
        if (selectedName) {
            $.each(_.where(allConfigs, {name: selectedName}), function (i, config) {
                res.Versions.push(config.version);
            });
            res.newConfigName(selectedName);
        }
        else {
            graph.clear();
            res.graphChanged(false);
        }
    });

    // Обработчик события выбора версии
    res.versionSelected.subscribe(function (version) {
        ShowMainSchema(version);
    });

    // Обработчик события выбора модуля
    res.selectedCell.subscribe(function (cell) {
        if (cell) {
            res.instnameSelectedModule(cell.attributes.attrs[".label"].text);

            // Определение специфичных для модуля полей, и установка их текущих значений
            // Очищаем информацию о параметрах (для визуализации) инстанса
            res.instanceSpecificProperties.removeAll();
            // Получаем метаданные для типа инстанса
            var moduleMeta = GetModuleMeta(cell.attributes.moduleType);

            // Получаем значения параметров инстанса
            var specificParams = GetInstanceSpecificParams(res.instnameSelectedModule(), moduleMeta);

            // Присваиваем значения параметров инстанса переменным, что будут учавствовать в биндинге
            if (moduleMeta.definition().paramsDefinitions) {
                moduleMeta.definition().paramsDefinitions.forEach(function (prop) {
                    prop.value = ko.observable(specificParams[prop.name]);

                    // Подписываемся на изменения значения параметра, указывая в качестве контекста specificParams
                    prop.value.subscribe(function (newValue) {
                        this.params[this.propertyName] = newValue;
                        res.graphChanged(true);
                    }, {propertyName: prop.name, params: specificParams});

                    res.instanceSpecificProperties.push(prop);
                });
            }

            // Установка значений параметров (общих для всех типов модулей) инстанса
            var commonParams = GetInstanceCommonParams(res.instnameSelectedModule(), moduleMeta);
            if (commonParams) {
                res.instanceCommonProperties().forEach(function (prop) {
                    // Отменим предыдущцю подписку, если таковая была
                    if (prop.subscription) {
                        prop.subscription.dispose();
                    }
                    // Установим текущее значение
                    prop.value(commonParams[prop.name]);

                    // Подписываемся на изменения значений свойств, указывая в качестве контекста commonParams
                    prop.subscription = prop.value.subscribe(function (newValue) {
                        this.params[this.metaProperty.name] = newValue;
                        res.graphChanged(true);
                    }, {metaProperty: prop, params: commonParams});
                });
            }
        }
    });

    graph.on('change', function () {
        res.graphChanged(true);
    });

    paper.on('cell:pointerdown', function (cellView, evt, x, y) {
        if (cellView.model.attributes.type == "devs.Model") {
            res.selectedCell(cellView.model);
            // Вызов контекстного меню по правой кнопке мыши
            if (evt.button == 2) {
                //console.log("press " + x + " " + y);
                var paperPosition = $("#paper").position();
                $("#moduleContextMenu").css({
                    display: "block",
                    left: x + paperPosition.left,
                    top: y + paperPosition.top
                });
            }
        }
        else {
            if (cellView.model.attributes.type == "link") {
                if (evt.button == 2) {
                    var paperPosition = $("#paper").position();
                    $("#linkContextMenu").css({
                        display: "block",
                        left: x + paperPosition.left,
                        top: y + paperPosition.top
                    });
                }
                res.selectedLink(cellView);
            }
        }
    })

    paper.on('blank:pointerdown', function (evt, x, y) {
        if (evt.button == 0) {
            res.instnameSelectedModule("Properties");
            $("#moduleContextMenu").hide();
            $("#linkContextMenu").hide();
        }
    })

    paper.on('cell:pointerdblclick', function (cellView, x, y) {
        if (cellView.model.attributes.blocksJSON) {
            // Это модуль, имеющий сложную конфигурации в виде подсхемы с блоками
            // Сохраним текущее положение дел в объекте конфигурации
            res.currentConfig().jsonGraph = JSON.stringify(graph.toJSON());
            // Подгрузим визуальную схему подмодуля
            graph.fromJSON(cellView.model.attributes.blocksJSON);
            res.listModules.removeAll();
            res.editSchemaMode("sub");
            res.instnameSelectedModule("Properties");

            var metaModule = GetModuleMeta(cellView.model.attributes.moduleType);

            $.each(metaModule.definition().subSchema.blocksDefinitions, function (i, item) {
                var el = new MetaOfModule()
                    .definition(item)
                res.listModules.push(el);
            });

            res.selectedSuperModule(cellView.model);
        }
    })

    graph.on('all', function (eventName, cell) {
        //console.log(arguments);
    });

    graph.on('add', function (cell) {
        if (cell.attributes.type == "link") {
            // При создании нового линка, ему по умолчанию устанавливается тип и цвет.
            cell.attributes["mode"] = "queue";

            var moduleType = GetModuleTypeByGraphId(cell.attributes.source.id);
            var moduleDef=GetModuleMeta(moduleType).definition();
            var group = _.find(moduleDef.outputs, function (group) {
                return cell.attributes.source.port in group.Schema.properties;
            });

            cell.attributes["nameOutGroup"] = group.name;
            cell.attributes["portType"] = group.Schema.properties[cell.attributes.source.port].type;
        }
    });


    res.PreparedLinks = {};
    // Подготавливает список свзяей в виде объекта, в котором можно добыть связь по имени модуля и имени порта
    // Для удосбства, при приеме данных из вебсокетов (чтоб не искать каждый раз связь в графе)
    var PrepareListLinks =function(){
        res.PreparedLinks = {};
        graph.attributes.cells.models.forEach(function (cell) {
            if(cell.attributes.type=="link"){
                var sourceInstanceName = _.find(graph.attributes.cells.models, function (model) {
                    return model.id == cell.attributes.source.id;
                }).attributes.attrs[".label"].text;

                if(!res.PreparedLinks.hasOwnProperty(sourceInstanceName)){
                    res.PreparedLinks[sourceInstanceName] = {};
                }
                if(!res.PreparedLinks[sourceInstanceName][cell.attributes.source.port]){
                    res.PreparedLinks[sourceInstanceName][cell.attributes.source.port] = new Array();
                }
                res.PreparedLinks[sourceInstanceName][cell.attributes.source.port].push(cell);
            }
        });
    };


    var socketTelemetry;
    var socketHostsOut;
    res.Init = function(){
        // Пока не установдлно соединение веюсокета, кнопки старта и стопа будут красными
        res.cssClass4ButtonsRunStop('btn btn-danger');

        var host = window.document.location.host.replace(/:.*/, '');
        if (typeof MozWebSocket != "undefined") {
            socketTelemetry = new MozWebSocket('ws://' + host + ':7681/xxx', "telemetry-protocol");
            socketHostsOut = new MozWebSocket('ws://' + host + ':3000');
        } else {
            socketTelemetry = new ReconnectingWebSocket('ws://' + host + ':7681/xxx', "telemetry-protocol");
            socketHostsOut = new ReconnectingWebSocket('ws://' + host + ':3000');
        }
        socketTelemetry.binaryType = "arraybuffer";

        try {
            socketTelemetry.onopen = function() {
                document.getElementById("wsdi_status").style.backgroundColor = "#40ff40";
                document.getElementById("wsdi_status").textContent = " websocket connection opened ";

                res.cssClass4ButtonsRunStop('btn btn-success');

                Subscribe2Telemetry("subscribe");
            }

            socketTelemetry.onmessage =function got_packet(msg) {
                // De serialize it again
                var obj = BSON.deserialize(new Uint8Array(msg.data));
//console.log(obj);
                $.each(obj, function (port, value) {
                    if(port!="_from" && (obj["_from"] in viewModels.Editor.PreparedLinks) && (port in viewModels.Editor.PreparedLinks[obj["_from"]]))
                    {
                        viewModels.Editor.PreparedLinks[obj["_from"]][port].forEach(function(link){
                            link.label(0, {
                                position: .5,
                                attrs: {
                                    text: {
                                        text: value.toFixed(2),
                                        fill: 'white',
                                        'font-family': 'sans-serif'
                                    },
                                    rect: {
                                        stroke: '#3498DB',
                                        fill: '#3498DB',
                                        'stroke-width': 10,
                                        rx: 3,
                                        ry: 3
                                    }
                                }
                            });
                        });
                    }
                });
            }

            socketTelemetry.onclose = function(){
                document.getElementById("wsdi_status").style.backgroundColor = "#ff4040";
                document.getElementById("wsdi_status").textContent = " websocket connection CLOSED ";

                res.cssClass4ButtonsRunStop('btn btn-danger');
            }
        } catch(exception) {
            alert('<p>Error' + exception);
        }

        socketHostsOut.onmessage = function (event) {
            var resp = JSON.parse(event.data);

            switch (resp.process) {
                case 'OS':
                    res.XenoCPU(resp.data.proc + "%");
                    break;

                default:
                    switch (resp.type) {
                        case 'stdout':
                            var text = String.fromCharCode.apply(String, resp.data).replace(/\n/g, '<br/>');

                            var text = text.replace(/\x1b\[34m/g, '<span style="color: blue">');
                            var text = text.replace(/\x1b\[31m/g, '<span style="color: red">');
                            var text = text.replace(/\x1b\[0m/g, '</span>');

                            document.getElementById('host_out').innerHTML =text;
                            break;

                        case 'status':
                            res.hostStatus(resp.data);
                            document.getElementById('host_out').innerHTML = resp.data;
                            break;
                    }
                    break
            }
        };
    }

    return res;
})();


$(document).ready(function () {

    $.when(
        $.getJSON("metamodules",
            function (data) {
                viewModels.Editor.LoadMetaModules(data);
            }),
        $.getJSON("getconfigs",
            function (data) {
                viewModels.Editor.LoadConfigurations(data);
            }),
        $.getJSON("gethoststatus",
            function (data) {
                viewModels.Editor.hostStatus(data);
            })
    ).then(function (a1, a2) {
            //Request OK
            ko.applyBindings(viewModels.Editor);

            viewModels.Editor.Init();
        }, function (err1, err2) {
            alert("error server request");
        });

});

// Запрет показа стандартного меню, вызываемого по правой кнопке мыши
document.addEventListener("contextmenu", function (e) {
    e.preventDefault();
}, false);


