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
            autoLoad: true,
            model: 'RtConfigurator.model.Schema'
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
                    this.set({
                       m_currentSchema: m_curSchema
                    });
                }
                return m_curSchema;
            }
        }
    }

});