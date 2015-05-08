Ext.define('RtConfigurator.store.StoreMetaModules', {
    extend: 'Ext.data.Store',
    autoLoad: false,
    storeId: 'MetamodulesStore',

    model: 'RtConfigurator.model.MetaModule',

    proxy: {
        type: 'jsonp',
        url: location.protocol + '//' + location.hostname + ':4000/metamodules',

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
