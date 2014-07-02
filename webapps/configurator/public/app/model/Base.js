Ext.define('RtConfigurator.model.Base', {
    extend: 'Ext.data.Model',

    fields: [{
        name: '_id',
        type: 'string'
    },{
        name: 'name',
        type: 'string'
    }],

    idProperty: '_id',

    schema: {
        namespace: 'RtConfigurator.model',  // generate auto entityName
        proxy: {
            type: 'rest',
            url: '{entityName}',
            reader: {
                type: 'json'//,
                //rootProperty: '{entityName:lowercase}'
            }
        }
    }
});