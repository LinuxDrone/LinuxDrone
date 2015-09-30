/**
 * This class is the view model for the Main view of the application.
 */
Ext.define('RtConfigurator.view.main.MainModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.main',

    requires: [
        'RtConfigurator.store.StoreMetaModules'
    ],
    store: {
        type: 'metamodules'
    }
});