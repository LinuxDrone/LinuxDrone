/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.configurator.svgpanel.SvgPanelController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'RtConfigurator.view.configurator.dialogs.ImportSchemaDialog',
        'RtConfigurator.view.configurator.logpanel.LogContentPanel'
    ],

    alias: 'controller.svgpanel',

    socketTelemetry: undefined,
    socketHostsOut: undefined,
    BSON: bson().BSON,


    init: function () {
        var model = this.getView().getViewModel();

        // Подписываемся на факт изменения текущей схемы
        model.bind('{currentSchema}', this.onSwitchCurrentSchema, this);

        // Подписываемся на факт выбора модуля
        model.bind('{selectedCell}', this.onSwitchCurrentCell);

        model.bind('{selectedLink}', this.onSelectLink);


        // После создания папера, отрисуем на нем текущую схему
        model.bind('{paper}', function (paper) {
            var currentSchema = model.get('currentSchema');
            if (!currentSchema) return;
            this.getView().controller.onSwitchCurrentSchema(model.get('currentSchema'));
        });

        var refreshSchemaComboLists = this.RefreshSchemaComboLists;

        // После загрузки списка всех схем, проинициализируем список имен схем
        model.get('listSchemas').addListener('load', function (storeListSchemas) {
            refreshSchemaComboLists(model);
        });

        // меняем цвет лабела отражающего статус коннекта вебсокета телеметрии
        model.bind('{telemetrySocketConnected}', function (socketConnected) {
            if (socketConnected)
                this.getView().logPanel.getViewModel().set('telemetryLabelBackground', 'LightGreen');
            else
                this.getView().logPanel.getViewModel().set('telemetryLabelBackground', 'red');
        });

        // меняем цвет лабела отражающего статус коннекта вебсокета логов
        model.bind('{logSocketConnected}', function (socketConnected) {
            if (socketConnected)
                this.getView().logPanel.getViewModel().set('logLabelBackground', 'LightGreen');
            else
                this.getView().logPanel.getViewModel().set('logLabelBackground', 'red');
        });


        this.initWebSockets();
    },


    initWebSockets: function () {
        // Пока не установлено соединение вебсокета, кнопки старта и стопа будут красными
        //res.cssClass4ButtonsRunStop('btn btn-danger');
        var controller = this;
        var model = this.getView().getViewModel();
        var logPanel = this.getView().logPanel;

        var host = window.document.location.host.replace(/:.*/, '');
        if (typeof MozWebSocket != "undefined") {
            this.socketTelemetry = new MozWebSocket('ws://' + host + ':7681/xxx', "telemetry-protocol");
            this.socketHostsOut = new MozWebSocket('ws://' + host + ':3000');
        } else {
            this.socketTelemetry = new ReconnectingWebSocket('ws://' + host + ':7681/xxx', "telemetry-protocol");
            this.socketHostsOut = new ReconnectingWebSocket('ws://' + host + ':3000');
        }
        this.socketTelemetry.binaryType = "arraybuffer";

        try {
            this.socketTelemetry.onopen = function () {
                model.set('telemetrySocketConnected', true);
            };

            this.socketTelemetry.onmessage = function got_packet(msg) {
                // Deserialize it again
                var obj = controller.BSON.deserialize(new Uint8Array(msg.data));
                var PreparedLinks = model.get('PreparedLinks');

                $.each(obj, function (port, value) {
                    if (port != "_from" && (obj["_from"] in PreparedLinks) && (port in PreparedLinks[obj["_from"]])) {
                        PreparedLinks[obj["_from"]][port].forEach(function (link) {
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
            };

            this.socketTelemetry.onclose = function () {
                model.set('telemetrySocketConnected', false);
            };
        } catch (exception) {
            alert('<p>Error' + exception);
        }

        try {
            this.socketHostsOut.onopen = function () {
                model.set('logSocketConnected', true);
            };
            this.socketHostsOut.onclose = function () {
                model.set('logSocketConnected', false);
            };

            this.socketHostsOut.onmessage = function (event) {
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

                                //document.getElementById('host_out').innerHTML = text;

                                //RtConfigurator.view.configurator.logpanel.LogContentPanel
                                console.log(resp);
                                console.log(text);
                                // Найти панель для данного лога

                                var logContentPanel = logPanel.lookupReference(resp.process);
                                if(!logContentPanel){
                                    logContentPanel = Ext.create('RtConfigurator.view.configurator.logpanel.LogContentPanel', {
                                        title: resp.process,
                                        reference: resp.process
                                    });
                                    logPanel.add(logContentPanel).show();
                                }
                                var store = logContentPanel.getStore();
                                store.add({log:text});
                                if(store.count()>100){
                                    store.removeAt(0);
                                }

                                break;

                            case 'status':
                                var currentSchema = model.get('currentSchema');
                                if (resp.data === 'stopped') {
                                    // Выполнение конфигурации остановлено.
                                    // Следует отписаться от телеметрии
//                                    controller.AllSubscribe2Telemetry(currentSchema, controller, 'unsubscribe');
                                } else {
                                    console.log(resp.data);
                                }
                                //res.hostStatus(resp.data);
                                //document.getElementById('host_out').innerHTML = resp.data;
                                break;
                        }
                        break
                }
            };
        } catch (exception) {
            alert('<p>Error' + exception);
        }
    },

    // Подписывание\отписывание на телеметрию всех инстансов схемы
    // type_subscribe = 'subscribe' или ''unsubscribe
    AllSubscribe2Telemetry: function (currentSchema, controller, type_subscribe) {
        $.each(currentSchema.get('modulesParams'), function (instanceName, params) {
            $.each(params.telemetrySubscriptions, function (outName, subscription) {
                if (subscription) {
                    controller.Subscribe2Telemetry(type_subscribe, instanceName, outName);
                    return;
                }
            });
        });
    },

    // Приватная. Подписаться, отписаться на телеметрию всех инстансов
    Subscribe2Telemetry: function (cmd, instanceName, outputName) {
        if (this.socketTelemetry.readyState == 1) {
            var obj = {
                cmd: cmd,
                instance: instanceName,
                out: outputName
            };
            var data = this.BSON.serialize(obj, true, true, false);
            this.socketTelemetry.send(data.buffer);
        }
    },

    onSelectLink: function (linkCell) {
        var configuratorModel = this.getView().ownerCt.getViewModel();
        if (!linkCell) {
            configuratorModel.set('hideLinkProperties', true);
        } else {
            var link = linkCell.model;
            // Значение в комбобоксе выбора типа связи в панели свойств связи
            configuratorModel.set('typeSelectedLink', link.attributes.mode);

            // Заголовок панели свойств
            configuratorModel.set('nameOfSelectedInstance', link.attributes.source.port + '->' + link.attributes.target.port);

            // Показать панель свойств связи
            configuratorModel.set('hideLinkProperties', false);

            // Обнулим ссылку на выбранный инстанс
            var model = this.getView().getViewModel();
            model.set('selectedCell', null);
        }
    },


    // Реврешит содержимое списков комбобоксов выбора схемы
    RefreshSchemaComboLists: function (svgPanelModel) {
        var storeListSchemas = svgPanelModel.get('listSchemas');

        if (storeListSchemas.count() == 0) {
            // Если база конфигураций пуста
            var newSchema = Ext.create('RtConfigurator.model.Schema', {
                name: 'New',
                version: 1,
                current: true
            });
            storeListSchemas.add(newSchema);
        }

        var namesStore = svgPanelModel.get('listSchemasNames');
        namesStore.removeAll();
        storeListSchemas.collect('name').forEach(function (entry) {
            namesStore.add({name: entry});
        });


        // Найдем и установим текущую схему
        var curSchema = storeListSchemas.findRecord('current', true)
        if (!curSchema) {
            curSchema = storeListSchemas.first();
        }

        var versionsStore = svgPanelModel.get('listSchemasVersions');
        // Отфильтруем список версий в соответствии с выбранным именем схемы
        versionsStore.removeFilter('name');
        versionsStore.addFilter([
            {
                property: 'name',
                value: curSchema.get('name'),
                operator: '='
            }
        ]);

        svgPanelModel.set('currentSchema', curSchema);
    },

    AddModule2Scheme: function (metaOfModule) {
        var model = this.getView().getViewModel();
        model.get('graph').addCell(this.MakeVisualModule(metaOfModule, model.get('graph')));
    },

    svgColors: {
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
        this.GetInstanceParams(name4NewInstance, moduleInfo);

        // Добавление параметров, со значениями по умолчанию
        return new joint.shapes.devs.Model(module);
    },


    // Приватная функция
    // Возвращает объект - значения конфигурационных параметров инстанса модуля
    GetInstanceParams: function GetInstanceParams(instanceName, moduleMeta) {
        var model = this.getView().getViewModel();
        var modulesParams = model.get('currentSchema').get('modulesParams');
        if (!modulesParams) {
            modulesParams = {};
            model.get('currentSchema').set('modulesParams', modulesParams);
        }

        var moduleParams;
        var requireMakeDefaults = false;
        switch (moduleMeta.type) {
            case "module_def":
                if (!modulesParams[instanceName]) {
                    requireMakeDefaults = true;
                    modulesParams[instanceName] = {common: {}, specific: {}, telemetrySubscriptions: {}};
                }
                moduleParams = modulesParams[instanceName];
                break;

            case "block_def":
                // Если это не модуль а блок, поместим его параметры в иерархию соответсвующего родительского инстанса модуля
                var parentSuperModuleName = res.selectedSuperModule().attributes.attrs[".label"].text;
                if (!modulesParams[parentSuperModuleName].blocksConfig) {
                    modulesParams[parentSuperModuleName].blocksConfig = {};
                }

                if (!modulesParams[parentSuperModuleName].blocksConfig[instanceName]) {
                    requireMakeDefaults = true;
                    modulesParams[parentSuperModuleName].blocksConfig[instanceName] = {specific: {}};
                }
                moduleParams = modulesParams[parentSuperModuleName].blocksConfig[instanceName];
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

            // Теперь заполним подписки на телеметрию (по умолчанию - false)
            if (moduleMeta.outputs) {
                $.each(moduleMeta.outputs, function (i, out) {
                    moduleParams.telemetrySubscriptions[out.name] = false;
                });
            }
        }
        return moduleParams;
    },

    onClickZoomOut: function (b, e, eOpts) {
        //this.initWebSockets();

        var model = this.getView().getViewModel();
        model.paperScaleX -= 0.1;
        model.paperScaleY -= 0.1;
        model.get('paper').scale(model.paperScaleX, model.paperScaleY);
    },

    onClickZoomIn: function (b, e, eOpts) {
        var model = this.getView().getViewModel();
        model.paperScaleX += 0.1;
        model.paperScaleY += 0.1;
        model.get('paper').scale(model.paperScaleX, model.paperScaleY);
    },

    // обработчик выбора имени схемы в комбобоксе
    onSelectSchema: function (combo, records, eOpts) {
        var model = this.getView().getViewModel();
        var versionsStore = model.get('listSchemasVersions');

        // Отфильтруем список версий в соответствии с выбранным именем схемы
        versionsStore.removeFilter('name');
        versionsStore.addFilter([
            {
                property: 'name',
                value: records[0].get('name'),
                operator: '='
            }
        ]);

        // Установим в качестве текщуй схемы, первую попавшуюся версию схемы с выбранным в данный момент именем
        // Но перед эти пометим текущую схему как не активную
        model.get('currentSchema').set('current', false);
        var newCurrent = versionsStore.first();
        newCurrent.set('current', true);
        model.set('currentSchema', newCurrent);

        model.get('listSchemas').sync();
    },

    // обработчик выбора версии схемы в комбобоксе
    onSelectVersion: function (combo, records, eOpts) {
        var model = this.getView().getViewModel();
        var storeListSchemas = model.get('listSchemas');

        // Найдем в хранилище схему с указанными именем и версией
        var ind = storeListSchemas.findBy(function (record, id) {
            return record.get('version') == records[0].get('version') && record.get('name') == model.get('currentSchema').get('name');
        });

        // Но перед эти пометим текущую схему как не активную
        model.get('currentSchema').set('current', false);

        // Установим в качестве текущей схемы, схемы с выбранной версией
        var newCurrent = storeListSchemas.getAt(ind);
        newCurrent.set('current', true);

        model.set('currentSchema', newCurrent);

        storeListSchemas.sync();
    },

    GetLocale: function () {
        var l_lang;
        if (navigator.userLanguage) // Explorer
            l_lang = navigator.userLanguage;
        else if (navigator.language) // FF
            l_lang = navigator.language;
        else
            l_lang = "en";
        return l_lang;
    },


    // Пытается вернуть текстовое свойство объекта в локали браузера
    Geti18nProperty: function (obj, propName) {
        if (obj.hasOwnProperty(propName)) {
            if (obj[propName].hasOwnProperty(this.getView().getController().GetLocale())) {
                return obj[propName][this.getView().getController().GetLocale()];
            }
            else {
                if (obj[propName].hasOwnProperty("en")) {
                    return obj.description.en;
                }
            }
        }
        return "";
    },

    // Приватная
    // Возвращает имя типа модуля по идентификатору модуля в графической схеме
    GetModuleTypeByGraphId: function (IdModelInGraph) {
        var model = this.getView().getViewModel();
        var graph = model.get('graph');

        return _.find(graph.attributes.cells.models,function (model) {
            return model.id == IdModelInGraph;
        }).attributes.moduleType;
    },


    // Обработчик выбора модуля на схеме
    onSwitchCurrentCell: function (cell) {
        var configuratorModel = this.getView().ownerCt.getViewModel();
        if (!cell) {
            configuratorModel.set('hideInstanceProperties', true);
        } else {
            var model = this.getView().getViewModel();

            var currentSchema = model.get('currentSchema');

            var instanceName = cell.attributes.attrs[".label"].text;
            var moduleParams = currentSchema.get('modulesParams')[instanceName];

            configuratorModel.set('currentModuleProps', moduleParams);
            configuratorModel.set('nameOfSelectedInstance', instanceName);

            // Показать панель свойств инстанса
            configuratorModel.set('hideInstanceProperties', false);

            // Обнулим ссылку на выбранную связь
            model.set('selectedLink', null);
        }
    },

    // Обработчик смены текущей схемы на другую
    onSwitchCurrentSchema: function (curSchema) {
        var model = this.getView().getViewModel();

        var graph = model.get('graph');

        if (!graph) {
            return;
        }

        var jsonGraph = curSchema.get('jsonGraph');
        if (jsonGraph) {
            graph.fromJSON(JSON.parse(jsonGraph));

            var paper = this.getView().getViewModel().get('paper');
            var controller = this.getView().getController();
            // Инициализация портов на модулях, с целью показа их описания в тултипе
            var elements = graph.getElements();
            $.each(elements, function (i, element) {
                var view = paper.findViewByModel(element);
                $.each([2, 3], function (i, s) {
                    $.each(view.el.childNodes[0].childNodes[s].childNodes, function (k, port) {
                        port.childNodes[0].onmouseenter = function (e) {
                            var groupsPorts = new Array();
                            var moduleType = element.attributes.moduleType;
                            var portName = port.textContent;

                            var storeMetamodules = Ext.data.StoreManager.lookup('StoreMetaModules');
                            var meta = storeMetamodules.findRecord('name', moduleType);
                            if (s == 2) {
                                groupsPorts.push(meta.get('inputShema').properties);
                            }
                            else {
                                $.each(meta.get('outputs'), function (i, m) {
                                    groupsPorts.push(m.Schema.properties);
                                });
                            }

                            var toolTipText = '';
                            $.each(groupsPorts, function (i, gp) {
                                var propsObj = gp;
                                if (propsObj.hasOwnProperty(portName)) {
                                    var portMeta = propsObj[portName];
                                    toolTipText = portMeta.type;
                                    toolTipText += "<br>" + controller.Geti18nProperty(portMeta, "description");
                                }
                            });

                            var tip = Ext.create('Ext.tip.ToolTip', {
                                target: e.currentTarget,
                                html: toolTipText
                            });
                        };
                    });
                });
            });
        }

        model.set('schemaChanged', false);

        //res.instnameSelectedModule("Properties");
//console.log(this);
        this.PrepareListLinks();
    },

    // Подготавливает список свзяей в виде объекта, в котором можно добыть связь по имени модуля и имени порта
    // Для удосбства, при приеме данных из вебсокетов (чтоб не искать каждый раз связь в графе)
    PrepareListLinks: function () {
        var model = this.getView().getViewModel();
        var PreparedLinks = {};
        var graph = model.get('graph');
        graph.attributes.cells.models.forEach(function (cell) {
            if (cell.attributes.type == "link") {
                var sourceInstanceName = _.find(graph.attributes.cells.models,function (model) {
                    return model.id == cell.attributes.source.id;
                }).attributes.attrs[".label"].text;

                if (!PreparedLinks.hasOwnProperty(sourceInstanceName)) {
                    PreparedLinks[sourceInstanceName] = {};
                }
                if (!PreparedLinks[sourceInstanceName][cell.attributes.source.port]) {
                    PreparedLinks[sourceInstanceName][cell.attributes.source.port] = new Array();
                }
                PreparedLinks[sourceInstanceName][cell.attributes.source.port].push(cell);
            }
        });
        model.set('PreparedLinks', PreparedLinks);
    },


    // Обработчик кнопки сохранения схемы
    onClickSaveSchema: function () {
        var currentSchema = this.getView().getViewModel().get('currentSchema');
        this.SaveCurrentConfig(currentSchema.get('name'), currentSchema.get('version'), false);
    },


    // Обработчик кнопки сохраненить схему как
    onClickOpenSaveAsSchemaDialog: function () {
        var model = this.getView().getViewModel();
        var configuratorModel = this.getView().ownerCt.getViewModel();
        configuratorModel.set('newSchemaName', model.get('currentSchemaName'));

        var newSaveAsDialog = Ext.create('RtConfigurator.view.configurator.dialogs.SaveAsSchemaDialog', {
            ownerCt: this.getView().ownerCt
        });
        newSaveAsDialog.show();
    },


    // Обработчик кнопки импортировать схему из файла
    onClickOpenImportSchemaDialog: function () {
        var newSaveAsDialog = Ext.create('RtConfigurator.view.configurator.dialogs.ImportSchemaDialog', {
            ownerCt: this.getView().ownerCt
        });
        newSaveAsDialog.show();
    },


    SaveAsSchema: function (newSchemaName, newSchemaVersion) {
        var model = this.getView().getViewModel();
        var storeListSchemas = model.get('listSchemas');

        var newSchema = Ext.create('RtConfigurator.model.Schema', {
            name: newSchemaName,
            version: newSchemaVersion,
            current: true
        });
        storeListSchemas.add(newSchema);

        var graph = model.get('graph');
        newSchema.set('jsonGraph', JSON.stringify(graph.toJSON()));

        // Текущая схема перестает быть таковой
        var currentSchema = model.get('currentSchema');
        currentSchema.set('current', false);

        var origModulesParams = currentSchema.get('modulesParams');

        // Копируем modulesParams из текущей схемы в новую
        newSchema.set('modulesParams', Ext.JSON.decode((Ext.JSON.encode(origModulesParams))));

        storeListSchemas.sync();
        this.RefreshSchemaComboLists(model);
    },

    AddNewSchema: function (newSchema) {
        var model = this.getView().getViewModel();
        var storeListSchemas = model.get('listSchemas');

        delete newSchema._id;
        newSchema.current = true;
        storeListSchemas.add(newSchema);

        // Текущая схема перестает быть таковой
        var currentSchema = model.get('currentSchema');
        currentSchema.set('current', false);

        storeListSchemas.sync();
        this.RefreshSchemaComboLists(model);
    },

    onClickDeleteSchema: function () {
        var model = this.getView().getViewModel();
        var listSchemas = model.get('listSchemas');
        var currentSchema = model.get('currentSchema');
        listSchemas.remove(currentSchema);
        listSchemas.sync();

        this.RefreshSchemaComboLists(model);
    },

    // Приватная функция сохранения конфигурации (текущего графа) с именем и версией
    SaveCurrentConfig: function (name, version, isNew) {
        var model = this.getView().getViewModel();
        var graph = model.get('graph');
        var storeListSchemas = model.get('listSchemas');
        // Найдем в хранилище схему с указанными именем и версией
        var ind = storeListSchemas.findBy(function (record, id) {
            return record.get('version') == version && record.get('name') == name;
        });
        var rec = storeListSchemas.getAt(ind);
        rec.set('jsonGraph', JSON.stringify(graph.toJSON()) + ' ');
        rec.set('date', new Date());
        rec.modified['modulesParams'] = rec.get('modulesParams');

        storeListSchemas.sync(
            {
                callback: function (batch, options) {
                    model.set('schemaChanged', false);
                }
            });
    },

    handleFileSelect: function (evt) {
        var files = evt.target.files; // FileList object

        // files is a FileList of File objects. List some properties.
        var output = [];
        for (var i = 0, f; f = files[i]; i++) {
            output.push('<li><strong>', escape(f.name), '</strong> (', f.type || 'n/a', ') - ',
                f.size, ' bytes, last modified: ',
                f.lastModifiedDate ? f.lastModifiedDate.toLocaleDateString() : 'n/a',
                '</li>');
        }
        //document.getElementById('list').innerHTML = '<ul>' + output.join('') + '</ul>';
    },

    RunConfig: function () {
        var model = this.getView().getViewModel();
        var currentSchema = model.get('currentSchema');
        var controller = this;

        var data4send = {
            "name": currentSchema.get('name'),
            "version": currentSchema.get('version')
        };
        $.post("runhosts", data4send,
            function (data) {
                // Выполнение конфигурации начато.
                // Следует подписаться на телеметрию
                controller.AllSubscribe2Telemetry(currentSchema, controller, 'subscribe');
            });
    },

    // Публичная функция остановки текущей конфигурации
    StopConfig: function () {
        var model = this.getView().getViewModel();
        var currentSchema = model.get('currentSchema');
        var controller = this;

        var data4send = {
            "name": currentSchema.get('name'),
            "version": currentSchema.get('version')
        };
        $.post("stophosts", data4send,
            function (data) {
                // Выполнение конфигурации остановлено.
                // Следует отписаться от телеметрии
                controller.AllSubscribe2Telemetry(currentSchema, controller, 'unsubscribe');
            });
    }
})
;
