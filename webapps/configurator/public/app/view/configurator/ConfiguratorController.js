/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.ConfiguratorController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'RtConfigurator.view.configurator.SaveAsSchemaDialog'
    ],

    alias: 'controller.configurator',

    init: function() {
    },


    onClickSaveAsSchema: function(){
        var currentSchema = this.getView().getViewModel().get('currentSchema');

        alert('sss');

        //this.SaveCurrentConfig(currentSchema.get('name'), currentSchema.get('version'), false);
    },


});
