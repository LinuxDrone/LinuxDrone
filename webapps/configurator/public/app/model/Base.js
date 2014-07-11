Ext.define('RtConfigurator.model.Base', {
    extend: 'Ext.data.Model',

    identifier: 'mongoidgenerator',

    fields: [{
        name: '_id',
        defaultValue: null
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