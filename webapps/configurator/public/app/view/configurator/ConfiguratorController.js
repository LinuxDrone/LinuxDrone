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


    // Вызывается при нажатии кнопки Save в диалоге SaveAs (SaveAsSchemaDialog)
    onClickSaveAsSchema: function(){
        var model = this.getView().getViewModel();
        var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];
        var svgpanelcontroller = svgpanelmodel.getView().controller;
        svgpanelcontroller.SaveAsSchema(model.get('newSchemaName'), model.get('newSchemaVersion'));
    }


});
