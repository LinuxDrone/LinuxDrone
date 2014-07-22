/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.ConfiguratorModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.configurator',

    data: {
        newSchemaName: '',
        newSchemaVersion: '',
        nameOfSelectedInstance: 'Properties',

        currentModuleProps: {}
    },

    formulas: {
        disableDeleteModule: function (get) {
            return get('nameOfSelectedInstance') == 'Properties';
        }
    }

});