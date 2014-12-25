Ext.define('RtConfigurator.store.StoreMetaModules', {
    extend: 'Ext.data.Store',
    autoLoad: true,
    storeId: 'MetamodulesStore',

    model: 'RtConfigurator.model.MetaModule',

    proxy: {
        type: 'jsonp',
        url: location.protocol + '//' + location.hostname + ':4000/metamodules',
        listeners : {
            exception : function(proxy, response, operation) {
                alert(operation.getError().statusText);
            }
        }
    }
});
