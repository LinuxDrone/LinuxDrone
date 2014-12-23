Ext.define('RtConfigurator.store.StoreMetaModules', {
    extend: 'Ext.data.JsonStore',
    autoLoad: true,
    storeId: 'MetamodulesStore',

    model: 'RtConfigurator.model.MetaModule',

    proxy: {
        type: 'rest',
        url: '/q/metamodules',
        listeners : {
            exception : function(proxy, response, operation) {
                alert('Got Exception....');
            }
        }
    }
});

