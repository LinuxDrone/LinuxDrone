/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.svg.SvgPanelModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.svg',

    paperScaleX: 1,
    paperScaleY: 1,

    stores: {
        // Список всех схем
        listSchemas : {
            autoLoad: true,
            model: 'RtConfigurator.model.Schema',
            proxy: {
                type: 'rest',
                reader : {
                    root:'data',
                    successProperty: 'success',
                    messageProperty : 'message'
                },
                api: {
                    read: 'getconfigs',
                    update: 'saveconfig',
                    create: 'newconfig',
                    destroy: 'delconfig'
                },
                listeners : {
                    exception : function(proxy, response, operation) {
                            Ext.MessageBox.show({
                                title: 'REMOTE EXCEPTION',
                                msg: operation.getError(),
                                icon: Ext.MessageBox.ERROR,
                                buttons: Ext.Msg.OK
                            });
                    }
                }
            },
            listeners:{
                write: function( store, operation, eOpts ){
                    // TODO: Здесь найти модель и установить schemaChanged в false
                    //alert("okkk");
                }
            }
        },

        // Список имен схем
        listSchemasNames : new Ext.data.ArrayStore({
            fields:['name']
        }),

        // Список версий для выбранного имени схемы
        listSchemasVersions : {
            source: '{listSchemas}'
        }
    },

    data: {
        graph: undefined,

        paper: undefined,

        // Текущая выбранная схема
        currentSchema:undefined,

        // Сигнализирует о том, что схема изменилась (изменился graph - визуальное представление)
        schemaChanged: false
    },

    formulas:{
        // Имя текущей схемы
        currentSchemaName:function(get){
            var curSchema = get('currentSchema');
            if(curSchema){
                return curSchema.get('name');
            }
        },

        // Версия текущей схемы
        currentSchemaVersion:function(get){
            var curSchema = get('currentSchema');
            if(curSchema){
                return curSchema.get('version');
            }
        }
    }
});