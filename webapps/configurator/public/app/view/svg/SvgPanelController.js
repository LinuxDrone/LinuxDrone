/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.svg.SvgPanelController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'Ext.MessageBox'
    ],

    alias: 'controller.svg',


    AddModule2Scheme: function(metaOfModule){
        var model = this.view.getViewModel();
        model.graph.addCell(this.MakeVisualModule(metaOfModule, model.graph));
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
        while (_.find(graph.getElements(), function (el) {
            return el.attributes.attrs['.label'] !== undefined && el.attributes.attrs['.label'].text === name4NewInstance;
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
        //this.GetInstanceParams(name4NewInstance, moduleInfo);

        // Добавление параметров, со значениями по умолчанию
        return new joint.shapes.devs.Model(module);
    },


    // Приватная функция
    // Возвращает объект - значения конфигурационных параметров инстанса модуля
    GetInstanceParams: function GetInstanceParams(instanceName, moduleMeta) {
        var moduleParams;
        var requireMakeDefaults = false;
        switch (moduleMeta.type) {
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
                alert("Unknown module object type '" + moduleMeta.type + "'");
        }

        if (requireMakeDefaults) {
            // Если для указанного инстанса нет в конфигурации параметров, следует их создать на основе дефолтных
            // из метаописания модуля
            if (moduleMeta.type == "module_def") {
                // Сначала заполним дефолтные значения для общих (для всех типов модулей) свойств
                $.each(_.pluck(ModulesCommonParams.commonModuleParamsDefinition, "name"), function (i, paramName) {
                    // Если дефолтное значение задано в определении модуля, то используем его
                    // Иначе (если опять же оно задано) возьмем его из общего для всех модулей определения
                    if (moduleMeta[paramName]) {
                        moduleParams.common[paramName] = moduleMeta[paramName];
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
            if (moduleMeta.paramsDefinitions) {
                $.each(moduleMeta.paramsDefinitions, function (i, paramDefinition) {
                    moduleParams.specific[paramDefinition.name] = paramDefinition.defaultValue;
                });
            }
        }
        return moduleParams;
    },

    onClickZoomOut: function (b, e, eOpts ){
        var model = this.view.getViewModel();
        model.paperScaleX -= 0.1;
        model.paperScaleY -= 0.1;
        model.paper.scale(model.paperScaleX, model.paperScaleY);
    },

    onClickZoomIn: function (b, e, eOpts ){
        var model = this.view.getViewModel();
        model.paperScaleX += 0.1;
        model.paperScaleY += 0.1;
        model.paper.scale(model.paperScaleX, model.paperScaleY);
    }

});
