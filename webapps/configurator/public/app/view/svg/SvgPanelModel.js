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
        firstName: 'John',
        lastName: 'Doe'
    },

    formulas: {
        listSchemas : function(get){
            var store = Ext.data.StoreManager.lookup('StoreSchemas');

            var res = new Array();
            store.collect('name').forEach(function(entry) {
                var rec = new Array();
                rec.push(entry);
                res.push(rec);
            });

            return new Ext.data.ArrayStore({
                fields: ['name'],
                data : res
                //data : [['Alabama'],['Alaska'],['Arizona']]
                //data : ['Alabama','Alaska','Arizona']
            });

        }
    }

});