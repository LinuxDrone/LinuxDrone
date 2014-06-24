/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.store.StoreSchemas', {
    extend: 'Ext.data.JsonStore',
    autoLoad: true,
    storeId: 'schemasStore',

    model: 'RtConfigurator.model.Schema',

    groupField: 'name',
    proxy: {
        type: 'rest',
        url: '/getconfigs'
    }
});

