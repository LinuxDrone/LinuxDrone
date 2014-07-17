/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.model.Schema', {
    extend: 'RtConfigurator.model.Base',

    fields: [
        {name: 'version', type: 'string'},
        {name: 'jsonGraph', type: 'string'},
        {name: 'modulesParams', type: 'auto' }
    ]
});