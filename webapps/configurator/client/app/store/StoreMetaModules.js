Ext.define('RtConfigurator.store.StoreMetaModules', {
    extend: 'Ext.data.Store',
    autoLoad: false,
    storeId: 'MetamodulesStore',
    alias: 'store.metamodules',

    model: 'RtConfigurator.model.MetaModule',

    proxy: {
        type: 'jsonp',
        // url задается при инициализации в файле LoginDialog
        listeners: {
            exception: function (proxy, request, operation, eOpts) {
                //console.log(operation);
                var errMsg = operation.getError();
                if (operation.getError().statusText) {
                    errMsg = operation.getError().statusText;
                }
                alert(errMsg);
            }
        }
    }
});
