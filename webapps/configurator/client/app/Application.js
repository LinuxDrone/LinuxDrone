/**
 * The main application class. An instance of this class is created by app.js when it calls
 * Ext.application(). This is the ideal place to handle application launch and initialization
 * details.
 */
Ext.define('RtConfigurator.Application', {
    extend: 'Ext.app.Application',

    requires: [
        'RtConfigurator.store.StoreMetaModules',
        'RtConfigurator.view.main.MainController',
        'RtConfigurator.view.configurator.dialogs.LoginDialog',
        'RtConfigurator.view.configurator.logpanel.LogPanel',
        'RtConfigurator.view.main.MainModel'
    ],

    name: 'RtConfigurator',

    views: [
        // TODO: add views here
    ],

    controllers: [
        //'Root'
    ],

    stores: [
        'StoreMetaModules'
    ],
    
    launch: function () {
        // TODO - Launch the application
        var loginDialog = Ext.create('RtConfigurator.view.configurator.dialogs.LoginDialog', {
            ownerCt: this._mainView
        });
        loginDialog.show();
    }
});
