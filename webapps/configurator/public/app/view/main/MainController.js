/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */
Ext.define('RtConfigurator.view.main.MainController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'Ext.MessageBox'
    ],

    alias: 'controller.main',

    onClickButton: function () {
        Ext.Msg.confirm('Confirm', 'Are you sure?', 'onConfirm', this);
    },

    onAddModule2Scheme: function(metaOfModule){
        var svgCanvas = this.lookupReference('svgCanvas');
        this.view.layout.centerRegion.setActiveTab(svgCanvas);

        svgCanvas.graph.addCell(this.MakeVisualModule(metaOfModule, svgCanvas.graph));

        /*
        if(svgCanvas.paper === undefined){
            svgCanvas.activate();
            Ext.Msg.confirm('Confirm', 'Добавить модуль?', 'onConfirm', this);
        }
        */
    },

    svgColors:{
        outPortsFillColor: '#E74C3C',
        normalModuleColor: '#2ECC71',
        smartModuleColor: '#CCCC71',
        blockColor: '#CC4C71',
        inPortsFillColor: '#16A085'
    },


// Приватная функция
    // Создает экзмепляр визуального модуля библиотеки joint, из описания модуля в формате linuxdrone
    MakeVisualModule: function MakeVisualModule(moduleInfo, graph) {
        var moduleDef = moduleInfo;

        var maxPins = 0;

        var module = {
            moduleType: moduleDef.name,
            position: { x: 10, y: 20 },
            size: { width: 90},
            attrs: {
                '.label': {'ref-x': .2, 'ref-y': -2 },
                rect: {fill: this.svgColors.normalModuleColor},
                '.inPorts circle': { fill: this.svgColors.inPortsFillColor },
                '.outPorts circle': { fill: this.svgColors.outPortsFillColor }
            }
        };


        switch (moduleDef.type) {
            case "module_def":
                if (moduleDef.subSchema) {
                    // Если это модуль сложноконфигурируемый и содержащий собственную подсхему
                    module.attrs.rect.fill = this.svgColors.smartModuleColor;
                    var graphBlocks = new joint.dia.Graph;
                    module.blocksJSON = graphBlocks.toJSON();
                }
                break;

            case "block_def":
                module.attrs.rect.fill = this.svgColors.blockColor;
                break;
        }


        var instancesCount = 1;
        var name4NewInstance = moduleDef.name + "-" + instancesCount;
        // Проверим не используется ли данное имя уже в качестве имени инстанса а схеме
/*
        while (_.find(graph.getElements(), function (el) {
            return el.attributes.attrs['.label'].text == name4NewInstance;
        })) {
            instancesCount++;
            name4NewInstance = moduleDef.name + "-" + instancesCount;
        }
*/

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
        //GetInstanceParams(name4NewInstance, moduleInfo);

        // Добавление параметров, со значениями по умолчанию
        return new joint.shapes.devs.Model(module);
    },


    onConfirm: function (choice) {
        if (choice === 'yes') {
            //
        }
    }
});
