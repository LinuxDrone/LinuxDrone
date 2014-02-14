var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: $('#paper'),
    gridSize: 20,
    model: graph
});
$(paper.el).find('svg').attr('preserveAspectRatio', 'xMinYMin');

var modulesDefs = {};

var viewModels = viewModels || {};


inputShema = [
    {
        name: "Task Priority",
        description: {ru: "Приоритет потока (задачи) xenomai"},
        defaultValue: 80,
        unitMeasured: "%",
        value: 80
    },
    {
        name: "Task Period",
        type: "number",
        required: true,
        description: {ru: "время между двумя вызовами бизнес функции (микросекунд) 0  - не зависать на очереди в ожидании данных -1 - зависать навечно, до факта появления данных в очереди"},
        defaultValue: 20,
        unitMeasured: "Ms",
        value: 20
    },
    {
        name: "Notify on change",
        type: "bool",
        unitMeasured: "",
        value: true
    }
]


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
        // Модуль выбранный на схеме
        selectedCell: ko.observable(),
        // Имя инстанса выбранного в схеме модуля
        instnameSelectedModule: ko.observable(''),
        // Общие свойства выбранного в схеме инстанса модуля
        moduleCommonProperties: ko.observableArray([])
    };


    inputShema.forEach(function (prop) {
        prop.value = ko.observable(prop.value);
        res.moduleCommonProperties.push(prop);
    });


    // Публичная функция загрузки конфигурации
    res.LoadConfigurations = function (_initialData) {
        allConfigs = _initialData;
        res.ConfigNames([]);
        $.each(_.groupBy(allConfigs, 'name'), function (configName, listVersions) {
            res.ConfigNames.push(configName);
        });
    };

    MetaOfModule = function () {
        var self = this;
        self.definition = ko.observable();
        self.instancesCount = ko.observable();
        self.AddModule2Paper = function () {
            graph.addCell(MakeVisualModule(self));
        };
    };

    // Публичная функция загрузки метаинформации модулей
    res.LoadMetaModules = function (_initialData) {
        res.metaModules([]);
        $.each(_initialData, function (i, item) {
            res.metaModules.push(new MetaOfModule()
                    .definition(item)
                    .instancesCount(0)
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
            res.selectedCell().remove();
            res.instnameSelectedModule("");
        }
    }

    res.chooseTemplate4Property = function chooseTemplate4Property(metaProperty) {
        if (metaProperty.type == "bool") {
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
            graph.fromJSON(JSON.parse(GetCurrentSchema(res.configNameSelected(), version).jsonGraph));
            res.graphChanged(false);
            res.instnameSelectedModule("");
        }
    });

    // Обработчик события выбора модуля
    res.selectedCell.subscribe(function (cell) {
        if (cell) {
            res.instnameSelectedModule(cell.attributes.attrs[".label"].text);

            var d = GetModuleParams(res.configNameSelected(), res.versionSelected(), res.instnameSelectedModule());


            res.moduleCommonProperties().forEach(function (prop) {
                // Подписываемся на изменения значений свойств, указывая в качестве контекста cell
                prop.value.subscribe(function (newValue) {
                    cell.commonProperies = cell.commonProperies || {};
                    cell.commonProperies[prop.name] = newValue;
                }, {metaProperty: prop, cell: cell});
            });
        }
    });

    // Приватная функция сохранения конфигурации (текущего графа) с именем и версией
    function SaveCurrentConfig(name, version, isNew) {
        var data4save = {
            "name": name,
            "version": version,
            "jsonGraph": JSON.stringify(graph.toJSON())
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
    // Возвращает объект текущей отображаемой схемы (из списка всех схем allConfigs)
    function GetCurrentSchema(schemaName, version) {
        return _.where(allConfigs, {name: schemaName, version: version})[0];
    }

    // Приватная функция
    // Возвращает объект - значения конфигурационных параметров инстанса модуля
    function GetModuleParams(schemaName, version, instanceName) {
        var vSchema = GetCurrentSchema(schemaName, version);
        vSchema.modulesParams = vSchema.modulesParams || {};

        if (!vSchema.modulesParams[instanceName]) {
            // Если для указанного инстанса нет в конфигурации параметров, следует их создать на основе дефолтных
            // из метаописания модуля
            vSchema.modulesParams[instanceName] = {};

            var d = 0;
        }

        return vSchema.modulesParams;
    }

    // Приватная функция
    // Возвращает описание модуля
    function GetModuleMeta(moduleName) {
        var vSchema = GetCurrentSchema(schemaName, version);
        vSchema.modulesParams = vSchema.modulesParams || {};

        if (!vSchema.modulesParams[instanceName]) {
            // Если для указанного инстанса нет в конфигурации параметров, следует их создать на основе дефолтных
            // из метаописания модуля
            vSchema.modulesParams[instanceName] = {};

            var d = 0;
        }

        return vSchema.modulesParams;
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
                '.inPorts circle': { fill: '#16A085' },
                '.outPorts circle': { fill: '#E74C3C' }
            }
        };

        moduleInfo.instancesCount(moduleInfo.instancesCount()+1);
        module.attrs['.label'].text = moduleDef.name + "-" + moduleInfo.instancesCount();

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


    graph.on('change', function () {
        res.graphChanged(true);
    });

    paper.on('cell:pointerdown', function (cellView, evt, x, y) {

        res.selectedCell(graph.findModelsFromPoint({x: x, y: y})[0]);

        if (evt.button == 2) {
            console.log("press " + x + " " + y);
        }

    })

    /*
     graph.on('all', function(type, param) {
     console.log(type);
     })
     */
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


