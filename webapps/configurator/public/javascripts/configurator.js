// Запрет показа стандартного меню, вызываемого по правой кнопке мыши
document.addEventListener("contextmenu", function (e) {
    e.preventDefault();
}, false);

inPortsFillColor = '#16A085';
outPortsFillColor = '#E74C3C';

var graph = new joint.dia.Graph;

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
    validateConnection: function (cellViewS, magnetS, cellViewT, magnetT, end, linkView) {
        //return true;
        // Prevent linking from input ports.
        if (magnetS && magnetS.attributes.fill.value === inPortsFillColor) return false;
        // Prevent linking from output ports to input ports within one element.
        if (cellViewS === cellViewT) return false;
        // Prevent linking to input ports.
        return magnetT && magnetT.attributes.fill.value === inPortsFillColor;
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

var viewModels = viewModels || {};

viewModels.Editor = (function () {
    var allConfigs = {};

    var res = {
        // Метаинформация модулей
        metaModules: ko.observableArray([]),
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

        // Общие свойства выбранного в схеме инстанса модуля
        instanceCommonProperties: ko.observableArray([]),
        // Специфические свойства выбранного в схеме инстанса модуля
        instanceSpecificProperties: ko.observableArray([])
    };

    ModulesCommonParams.commonModuleParamsDefinition.forEach(function (prop) {
        prop.value = ko.observable(prop.defaultValue);
        res.instanceCommonProperties.push(prop);
    });

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
            return "In the future there will be a description of the module";
        };
    };

    // Публичная функция загрузки метаинформации модулей
    res.LoadMetaModules = function (_initialData) {
        res.metaModules([]);
        $.each(_initialData, function (i, item) {
            res.metaModules.push(new MetaOfModule()
                    .definition(item)
            );
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

    res.RemoveModule = function RemoveModule() {
        if (res.selectedCell()) {
            // Следует так же удалить настройки модуля
            delete res.currentConfig().modulesParams[res.selectedCell().attributes.attrs['.label'].text];

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
    }

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
        if (version) {
            res.currentConfig(GetConfig(res.configNameSelected(), version));
            if (res.currentConfig().jsonGraph) {
                graph.fromJSON(JSON.parse(res.currentConfig().jsonGraph));

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
                                if(s==2)
                                {
                                    groupsPorts.push(meta.definition().inputShema.properties);
                                }
                                else
                                {
                                    $.each(meta.definition().outputs, function (i, m) {
                                        groupsPorts.push(m.Schema.properties);
                                    });
                                }

                                $.each(groupsPorts, function (i, gp) {
                                    var propsObj = gp;
                                    if(propsObj.hasOwnProperty(portName))
                                    {
                                        var portMeta = propsObj[portName];

                                        var text = portMeta.type;

                                        if(portMeta.hasOwnProperty("description"))
                                        {
                                            if(portMeta.description.hasOwnProperty(GetLocale()))
                                            {
                                                text += "<br>" + portMeta.description[GetLocale()];
                                            }
                                            else
                                            {
                                                if(portMeta.description.hasOwnProperty("en"))
                                                {
                                                    text += "<br>" + portMeta.description.en;
                                                }
                                            }
                                        }

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
        }
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
            var specificParams = GetInstanceSpecificParams(res.instnameSelectedModule(), cell.attributes.moduleType);

            // Присваиваем значения параметров инстанса переменным, что будут учавствовать в биндинге
            moduleMeta.definition().paramsDefinitions.forEach(function (prop) {
                prop.value = ko.observable(specificParams[prop.name]);

                // Подписываемся на изменения значения параметра, указывая в качестве контекста specificParams
                prop.value.subscribe(function (newValue) {
                    this.params[this.propertyName] = newValue;
                    res.graphChanged(true);
                }, {propertyName: prop.name, params: specificParams});

                res.instanceSpecificProperties.push(prop);
            });

            // Установка значений параметров (общих для всех типов модулей) инстанса
            var commonParams = GetInstanceCommonParams(res.instnameSelectedModule(), cell.attributes.moduleType);
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
    });

    // Приватная функция сохранения конфигурации (текущего графа) с именем и версией
    function SaveCurrentConfig(name, version, isNew) {
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
    }

    // Приватная функция
    // Возвращает объект текущего редактируемого конфига (из списка всех конфигов allConfigs)
    function GetConfig(configName, version) {
        var cfg = _.where(allConfigs, {name: configName, version: version})[0];
        cfg.modulesParams = cfg.modulesParams || {};
        //cfg.linksParams = cfg.linksParams || {};
        return cfg;
    }

    // Приватная функция
    // Возвращает объект - значения общих конфигурационных параметров инстанса модуля
    function GetInstanceCommonParams(instanceName, moduleType) {
        return GetInstanceParams(instanceName, moduleType).common;
    }

    // Приватная функция
    // Возвращает объект - значения общих конфигурационных параметров инстанса модуля
    function GetInstanceSpecificParams(instanceName, moduleType) {
        return GetInstanceParams(instanceName, moduleType).specific;
    }

    // Приватная функция
    // Возвращает объект - значения конфигурационных параметров инстанса модуля
    function GetInstanceParams(instanceName, moduleType) {
        if (!res.currentConfig().modulesParams[instanceName]) {
            // Если для указанного инстанса нет в конфигурации параметров, следует их создать на основе дефолтных
            // из метаописания модуля
            res.currentConfig().modulesParams[instanceName] = {common: {}, specific: {}};
            var moduleParams = res.currentConfig().modulesParams[instanceName];

            var moduleMeta = GetModuleMeta(moduleType);
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

            // Теперь установим специфичные для модуля параметры, взяв их значения из определения типа модуля
            $.each(moduleMeta.definition().paramsDefinitions, function (i, paramDefinition) {
                moduleParams.specific[paramDefinition.name] = paramDefinition.defaultValue;
            });
        }
        return res.currentConfig().modulesParams[instanceName];
    }

    // Приватная функция
    // Возвращает описание модуля
    function GetModuleMeta(moduleName) {
        return _.find(res.metaModules(), function (meta) {
            return meta.definition().name == moduleName;
        });
    }

    // Приватная функция
    // Создает экзмепляр визуального модуля библиотеки joint, из описания модуля в формате linuxdrone
    function MakeVisualModule(moduleInfo) {
        var moduleDef = moduleInfo.definition();

        var maxPins = 0;

        var module = {
            moduleType: moduleDef.name,
            position: { x: 10, y: 20 },
            size: { width: 90},
            attrs: {
                '.label': {'ref-x': .2, 'ref-y': -2 },
                rect: { fill: '#2ECC71' },
                '.inPorts circle': { fill: inPortsFillColor },
                '.outPorts circle': { fill: outPortsFillColor }
            }
        };

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
                if (maxPins < propsCount) {
                    maxPins = propsCount;
                }
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

        // Добавление параметров, со значениями по умолчанию
        return new joint.shapes.devs.Model(module);
    }

    function GetLocale()
    {
        var l_lang;
        if (navigator.userLanguage) // Explorer
            l_lang = navigator.userLanguage;
        else if (navigator.language) // FF
            l_lang = navigator.language;
        else
            l_lang = "en";
        return l_lang;
    }

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

    graph.on('all', function (eventName, cell) {
        //console.log(arguments);
    });

    graph.on('add', function (cell) {
        if (cell.attributes.type == "link") {
            // При создании нового линка, ему по умолчанию устанавливается тип и цвет.
            cell.attributes["mode"] = "queue";
        }
    })

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
            })
    ).then(function (a1, a2) {
            //Request OK
            ko.applyBindings(viewModels.Editor);
        }, function (err1, err2) {
            alert("error server request");
        });

});


