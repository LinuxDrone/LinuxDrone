/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.configurator.svgpanel.SvgPanelModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.svgpanel',

    paperScaleX: 1,
    paperScaleY: 1,

    stores: {
        // Список всех схем
        listSchemas: {
            autoLoad: true,
            model: 'RtConfigurator.model.Schema',
            proxy: {
                type: 'rest',
                reader: {
                    rootProperty: 'data',
                    successProperty: 'success',
                    messageProperty: 'message'
                },
                api: {
                    read: 'getconfigs',
                    update: 'saveconfig',
                    create: 'newconfig',
                    destroy: 'delconfig'
                },
                listeners: {
                    exception: function (proxy, response, operation) {
                        var errMsg = operation.getError();
                        if (operation.getError().statusText) {
                            errMsg = operation.getError().statusText;
                        }
                        Ext.MessageBox.show({
                            title: 'REMOTE EXCEPTION',
                            msg: errMsg,
                            icon: Ext.MessageBox.ERROR,
                            buttons: Ext.Msg.OK
                        });
                    }
                }
            },
            listeners: {
                write: function (store, operation, eOpts) {
                    // TODO: Здесь найти модель и установить schemaChanged в false
                    //alert("okkk");
                }
            }
        },

        // Список имен схем
        listSchemasNames: new Ext.data.ArrayStore({
            fields: ['name']
        }),

        // Список версий для выбранного имени схемы
        listSchemasVersions: {
            source: '{listSchemas}'
        }
    },

    data: {
        graph: undefined,

        paper: undefined,

        // Текущая выбранная схема
        currentSchema: undefined,

        // Сигнализирует о том, что схема изменилась (изменился graph - визуальное представление)
        schemaChanged: false,

        // Модуль выбранный на схеме
        selectedCell: undefined,

        // Связь выбранная на схеме
        selectedLink: undefined,

        // Состояние вебсокетов
        telemetrySocketConnected: false,
        logSocketConnected: false,
        // Статус запущенности процесса :-)
        started: false,

        // Подготовленный список свзяей в виде объекта, в котором можно добыть связь по имени модуля и имени порта
        // Для удосбства, при приеме данных из вебсокетов (чтоб не искать каждый раз связь в графе)
        PreparedLinks: undefined
    },

    formulas: {
        // Имя текущей схемы
        currentSchemaName: function (get) {
            var curSchema = get('currentSchema');
            if (curSchema) {
                return curSchema.get('name');
            }
        },

        // Версия текущей схемы
        currentSchemaVersion: function (get) {
            var curSchema = get('currentSchema');
            if (curSchema) {
                return curSchema.get('version');
            }
        },

        exportLink: function(get){
            var curSchema = get('currentSchema');
            return '<a target="_blank" href="getconfig/'+curSchema.id +'">Export</a>';
        }
    }
});