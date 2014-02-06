var allConfigs = {};
var modules = new Array();

$(document).ready(function () {
    $.getJSON("metamodules",
        function (data) {
            $.each(data, function (i, item) {

                var module = {
                    position: { x: 10, y: 20 },
                    size: { width: 90, height: 250 },
                    attrs: {
                        '.label': { text: 'Mpu6050', 'ref-x': .2, 'ref-y': -2 },
                        rect: { fill: '#2ECC71' },
                        '.inPorts circle': { fill: '#16A085' },
                        '.outPorts circle': { fill: '#E74C3C' }
                    }
                };

                module.attrs['.label'].text = item.name;

                if (item.outputs) {
                    module.outPorts = new Array();
                    $.each(item.outputs, function (i, output) {
                        $.each(output.Schema.properties, function (i, pin) {
                            module.outPorts.push(i);
                        });
                    });
                }

                if (item.inputShema) {
                    module.inPorts = new Array();
                    $.each(item.inputShema.properties, function (i, pin) {
                        module.inPorts.push(i);
                    });
                }

                modules.push(new joint.shapes.devs.Model(module));

                //console.log(module);
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


var graph = new joint.dia.Graph;

var paper = new joint.dia.Paper({
    el: $('#paper'),
    width: 1024,
    height: 400,
    gridSize: 20,
    model: graph
});


function InitListModules() {
    modules.forEach(function (entry) {
        var newButton = document.createElement("input");
        newButton.type = "button";
        newButton.value = entry.attributes.attrs[".label"].text;
        newButton.onclick = function () {
            AddModule(entry);
        };
        document.getElementById("ModulesPanel").appendChild(newButton);
    });
}


function AddModule(module) {
    graph.addCell(module.clone());
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
    else
    {
        graph.clear();
        $('#configName')[0].value="";
        $('#configVersion')[0].value="";
    }
}
