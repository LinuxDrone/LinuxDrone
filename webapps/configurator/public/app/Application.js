/**
 * The main application class. An instance of this class is created by app.js when it calls
 * Ext.application(). This is the ideal place to handle application launch and initialization
 * details.
 */
Ext.define('RtConfigurator.Application', {
    extend: 'Ext.app.Application',
    
    name: 'RtConfigurator',

    views: [
        // TODO: add views here
    ],

    controllers: [
        'Root'
    ],

    stores: [
        'StoreMetaModules',
        'StoreSchemas'
    ],
    
    launch: function () {
        // TODO - Launch the application
        //var store = Ext.data.StoreManager.lookup('StoreMetaModules');
    }
});
