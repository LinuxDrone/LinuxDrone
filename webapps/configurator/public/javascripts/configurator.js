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
            //console.log(data);
            allConfigs = data;
            $.each(data, function (i, item) {
                $('#listConfigs')[0].options[i] = new Option(item.name, i);
            });
            SelectConfig();
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

    module.size.height= maxPins * 30;

    // Добавление параметров, со значениями по умолчанию


    return new joint.shapes.devs.Model(module);
}


var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: $('#paper'),
    width: window.innerWidth - 220,
    height: window.innerHeight - 100,
    gridSize: 20,
    model: graph
});


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

function SaveConfig() {
    $.post("saveconfig",
        {
            "name": $('#configName')[0].value,
            "version": parseInt($('#configVersion')[0].value),
            "jsonGraph": JSON.stringify(graph.toJSON())
        },
        function (data) {
            alert("Data Loaded: " + data);
        });
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
    SelectConfig();

}

function SelectConfig() {
    if ($('#listConfigs')[0].options.length != 0) {
        $('#configName')[0].value = allConfigs[$('#listConfigs')[0].selectedIndex].name;
        $('#configVersion')[0].value = allConfigs[$('#listConfigs')[0].selectedIndex].version;
        graph.fromJSON(JSON.parse(allConfigs[$('#listConfigs')[0].selectedIndex].jsonGraph));
    }
    else {
        graph.clear();
        $('#configName')[0].value = "";
        $('#configVersion')[0].value = "";
    }
}

$(window).resize(function () {
    paper.setDimensions(window.innerWidth - 220, window.innerHeight - 100);
});