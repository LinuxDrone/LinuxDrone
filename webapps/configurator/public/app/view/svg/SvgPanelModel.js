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
                load: function(th) {
                    var store = Ext.data.StoreManager.lookup('StoreSchemas');
                    if(store){
                        store.collect('name').forEach(function(entry) {
                            th.add({name: entry});
                        });
                    }
                }
            }
        }),
        listSchemasVersions : new Ext.data.ArrayStore({
            //autoLoad: true,
            fields:['_id','version']
            /* ,
            listeners: {
                load: function(th) {
                    var store = Ext.data.StoreManager.lookup('StoreSchemas');
                    if(store){
                        store.collect('name').forEach(function(entry) {
                            th.add({version: '0'});
                        });
                    }
                }
            }
            */
        }),
        m_currentSchema:false
    },

    formulas: {
        currentSchema: {
            get: function (get) {
                var m_curSchema = get('m_currentSchema');
                if(!m_curSchema){
                    var store = Ext.data.StoreManager.lookup('StoreSchemas');
                    m_curSchema=store.findRecord('current', true);
                }
                return m_curSchema;
            },
            set: function (value) {
                var store = Ext.data.StoreManager.lookup('StoreSchemas');
                var group = store.getGroups().getByKey(value);

                var listVersions = this.data.listSchemasVersions;
                listVersions.removeAll();

                $.each(group.items, function (i, e) {
                    listVersions.add(e.data);
                });


                //this.set({
                  //  listSchemasVersions: group
                //});
            }
        },
        currentVersion:{
            get: function (get) {
                return '1';
            },
            set: function (value) {
/*
                this.set({
                    firstName: value.substring(0, split),
                    lastName: value.substring(split + 1)
                });
*/
                alert(value);
            }
        }
    }

});