Ext.define('RtConfigurator.model.Base', {
    extend: 'Ext.data.Model',

    fields: [{
        name: 'name',
        type: 'string'
    }],

    schema: {
        namespace: 'RtConfigurator.model',  // generate auto entityName

        proxy: {     // Ext.util.ObjectTemplate
            type: 'rest',
            url: '{entityName}',
            reader: {
                type: 'json'//,
                //rootProperty: '{entityName:lowercase}'
            }
        }
    }
});