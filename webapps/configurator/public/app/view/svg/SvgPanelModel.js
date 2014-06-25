/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.svg.SvgPanelModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.svg',

    graph: undefined,

    paper: undefined,

    store: 'schemasStore',

    paperScaleX: 1,
    paperScaleY: 1,

    data: {
        listSchemasNames : new Ext.data.ArrayStore({
            autoLoad: true,
            model: 'RtConfigurator.model.Schema',
            listeners: {
                load: function( th, records, successful, eOpts ) {
                    var store = Ext.data.StoreManager.lookup('StoreSchemas');
                    store.collect('name').forEach(function(entry) {
                        th.add({name: entry});
                    });
                }
            }
        })
    },

    formulas: {
        selectedSchemaName: {
            get: function (get) {
                var col = get('listSchemasNames');
                if(col.count()==0){
                    col.load();
                }
                return col.first().data.name;
            },

            set: function (value) {
                //var col = this.data.listSchemasNames;
                //col.add({name: 'dddddd'});
            }
        }
    }

});