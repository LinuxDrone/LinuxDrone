var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: $('#paper'),
    gridSize: 20,
    model: graph
});
$(paper.el).find('svg').attr('preserveAspectRatio', 'xMinYMin');


var viewModels = viewModels || {};

viewModels.ConfigurationSelector = function () {
    var self = this;
    self.name = ko.observable();
    self.listVersions = ko.observable();
};

viewModels.Editor = (function () {
    var res = {
        // Список имен конфигураций и ассоциированных с именем одной или нескольких версий конфигураций
        Configurations: ko.observableArray([]),
        // Выбранная (имя+ассоциированные версии) конфигурация
        configSelected: ko.observable(),
        // Список версий (конфигураций) для выбранного имени конфигурации
        Versions: ko.observableArray([]),
        // Выбранная версия (строка - назвение версии)
        versionSelected: ko.observable(),
        newConfigName: ko.observable(),
        newConfigVersion: ko.observable()
    };

    // Публичная функция загрузки конфигурации
    res.LoadConfigurations = function (_initialData) {
        // clear array if loading dynamic data
        res.Configurations([]);

        $.each(_initialData, function (configName, listVersions) {
            res.Configurations.push(new viewModels.ConfigurationSelector()
                .name(configName)
                .listVersions(listVersions)
            );
        });
    };

    // Публичная функция сохранения текущей конфигурации
    // Название и версия берутся из комбобоксов имени и версий
    res.SaveConfig = function SaveConfig() {
        SaveCurrentConfig(res.configSelected()[0].name, res.configSelected()[0].version);
    }

    // Публичная функция сохранения текущей конфигурации под новым именем
    // Название и версия берутся из полей ввода диалога "Сохранить как.."
    res.SaveConfigAs = function SaveConfigAs() {
        SaveCurrentConfig(res.newConfigName(), res.newConfigVersion());
    }

    // Обработчик события выбора имени конфигурации
    res.configSelected.subscribe(function (versions) {
        res.Versions([]);
        $.each(versions, function (i, configVersion) {
            res.Versions.push(configVersion.version);
        });
        res.newConfigName(versions[0].name);
    });

    // Обработчик события выбора версии
    res.versionSelected.subscribe(function (version) {
        if (version) {
            graph.fromJSON(JSON.parse(_.where(res.configSelected(), {version: version})[0].jsonGraph));
        }
    });


    // Приватная функция сохранения конфигурации (текущего графа) с именем и версией
    function SaveCurrentConfig(name, version) {
        $.post("saveconfig",
            {
                "name": name,
                "version": version,
                "jsonGraph": JSON.stringify(graph.toJSON())
            },
            function (data) {
                alert("Data Loaded: " + data);
            });
    }

    return res;
})();


var allConfigs = {};
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
            viewModels.Editor.LoadConfigurations(_.groupBy(data, 'name'));
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


function DeleteConfig() {

    $.post("delconfig",
        {
            "name": $('#configName')[0].value,
            "version": parseInt($('#configVersion')[0].value)
        },
        function (data) {
            alert("Data Loaded: " + data);
        });

    $('#listConfigs')[0].remove($('#listConfigs')[0].selectedIndex);
    //SelectConfig();

}


