/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.svg.SvgPanelController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'Ext.MessageBox'
    ],

    alias: 'controller.svg',

    init: function() {
        var model = this.getView().getViewModel();

        // Подписываемся на факт изменения текущей схемы
        model.bind('{currentSchema}', this.onChangeCurrentSchema);

        // После создания папера, отрисуем на нем текущую схему
        model.bind('{paper}', function(paper){
            this.getView().controller.onChangeCurrentSchema(model.get('currentSchema'));
        });


        // После загрузки списка всех схем, проинициализируем список имен схем
        model.get('listSchemas').addListener('load', function(storeListSchemas) {
            var namesStore = model.get('listSchemasNames');
            storeListSchemas.collect('name').forEach(function(entry) {
                namesStore.add({name: entry});
            });
            // Найдем и установим текущую схему
            model.set('currentSchema', storeListSchemas.findRecord('current', true));
        });
    },

    AddModule2Scheme: function(metaOfModule){
        var model = this.getView().getViewModel();
        model.get('graph').addCell(this.MakeVisualModule(metaOfModule, model.get('graph')));
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
        var model = this.getView().getViewModel();
        model.paperScaleX -= 0.1;
        model.paperScaleY -= 0.1;
        model.get('paper').scale(model.paperScaleX, model.paperScaleY);
    },

    onClickZoomIn: function (b, e, eOpts ){
        var model = this.getView().getViewModel();
        model.paperScaleX += 0.1;
        model.paperScaleY += 0.1;
        model.get('paper').scale(model.paperScaleX, model.paperScaleY);
    },

    onSelectSchema: function(combo, records, eOpts){
        var model=this.getView().getViewModel();
        var versionsStore = model.get('listSchemasVersions');

        // Отфильтруем список версий в соответствии с выбранным именем схемы
        versionsStore.removeFilter('name');
        versionsStore.addFilter([{
            property: 'name',
            value: records[0].get('name'),
            operator: '='
        }]);

        // Установим в качестве текщуй схемы, первую попавшуюся версию схемы с выбранным в данный момент именем
        model.set('currentSchema', versionsStore.first());
    },

    onSelectVersion: function(combo, records, eOpts){
        var model = this.getView().getViewModel();
        var storeListSchemas = model.get('listSchemas');

        // Найдем в хранилище схему с указанными именем и версией
        var ind = storeListSchemas.findBy(function(record, id){
            return record.get('version') == records[0].get('version') && record.get('name') == model.get('currentSchema').get('name');
        });

        // Установим в качестве текущей схемы, схемц с выбранной версией
        model.set('currentSchema', storeListSchemas.getAt(ind));
    },

    onChangeCurrentSchema: function(curSchema){
        console.log(this.getView().getViewModel());

        var graph = this.getView().getViewModel().get('graph');

        if(!graph){
            return;
        }


        var jsonGraph = curSchema.get('jsonGraph');
        if (jsonGraph) {
            graph.fromJSON(JSON.parse(jsonGraph));

            /*
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
            */
        }
        //res.graphChanged(false);
        //res.instnameSelectedModule("Properties");

        //PrepareListLinks();
    }

});
