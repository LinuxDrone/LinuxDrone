var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: $('#paper'),
    gridSize: 20,
    model: graph
});
$(paper.el).find('svg').attr('preserveAspectRatio', 'xMinYMin');


var viewModels = viewModels || {};

/*
 viewModels.ConfigurationSelector = function () {
 var self = this;
 self.name = ko.observable();
 self.listVersions = ko.observableArray([]);
 };
 */

viewModels.Editor = (function () {
    var allConfigs = {};

    var res = {
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

        selectedCell: ko.observable(),
        instnameSelectedModule: ko.observable('')
    };

    // Публичная функция загрузки конфигурации
    res.LoadConfigurations = function (_initialData) {
        allConfigs = _initialData;

        res.ConfigNames([]);

        $.each(_.groupBy(allConfigs, 'name'), function (configName, listVersions) {
            res.ConfigNames.push(configName);
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
            graph.fromJSON(JSON.parse(_.where(allConfigs, {version: version, name: res.configNameSelected()})[0].jsonGraph));
            res.graphChanged(false);
            res.instnameSelectedModule("");
        }
    });

    // Обработчик события выбора версии
    res.selectedCell.subscribe(function (cell) {
        if (cell) {
            res.instnameSelectedModule(cell.attributes.attrs[".label"].text);
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
                        //var names = _.where(res.Configurations(), {"name()":name});
                        //var existConfig = _.find(res.Configurations(), function(confSel){ return confSel.name() == name; });
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
                    else
                    {
                        allConfigs = _.reject(allConfigs, function (cfg) {
                            return cfg.name == name && cfg.version == version;
                        });
                        allConfigs.push(data4save);
                    }
                    res.graphChanged(false);
                }
            });
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


var modulesDefs = {};

$(document).ready(function () {
    $.getJSON("metamodules",
        function (data) {
            $.each(data, function (i, item) {
                modulesDefs[item.name] = {definition: item, instancesCount: 0};
            });

            InitListModules();
        });

    $.getJSON("getconfigs",
        function (data) {
            viewModels.Editor.LoadConfigurations(data);
            ko.applyBindings(viewModels.Editor);
        });
});

// Создает экзмепляр визуального модуля библиотеки joint, из описания модуля в формате linuxdrone
function MakeVisualModule(moduleInfo) {
    var moduleDef = moduleInfo.definition;

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

    moduleInfo.instancesCount++;
    module.attrs['.label'].text = moduleDef.name + "-" + moduleInfo.instancesCount;

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


function InitListModules() {
    var modulesNames = Object.keys(modulesDefs);
    modulesNames.forEach(function (i) {
        var moduleDef = modulesDefs[i].definition;
        var newButton = document.createElement("input");
        newButton.type = "button";
        //newButton.value = entry.attributes.attrs[".label"].text;
        newButton.value = moduleDef.name + " (v" + moduleDef.version + ")";
        newButton.style.margin = "2px";
        newButton.onclick = function () {
            AddModule2Paper(modulesDefs[i]);
        };
        document.getElementById("ModulesPanel").appendChild(newButton);
    });
}


function AddModule2Paper(moduleDef) {
    graph.addCell(MakeVisualModule(moduleDef));
}