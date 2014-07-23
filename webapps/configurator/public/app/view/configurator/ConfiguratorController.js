/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.ConfiguratorController', {
    extend: 'Ext.app.ViewController',

    requires: [
        'RtConfigurator.view.configurator.SaveAsSchemaDialog'
    ],

    alias: 'controller.configurator',

    init: function () {
        var model = this.getView().getViewModel();

        // Подписываемся на факт изменения текущей схемы
        model.bind('{currentModuleProps}', this.onChangeCurrentModuleProps);

        // При обновлении параметров модуля, пометить схему как измененную.
        this.getView().lookupReference('commonProperties').getStore().on('update', this.markSchemaAsChanged, this);
        this.getView().lookupReference('specificProperties').getStore().on('update', this.markSchemaAsChanged, this);
        this.getView().lookupReference('telemetrySelect').getStore().on('update', this.markSchemaAsChanged, this);
    },

    markSchemaAsChanged: function(store, record, operation, modifiedFieldNames, eOpts){
        if(operation=='edit'){
            var model = this.getView().getViewModel();
            model.children['rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1'].set('schemaChanged', true);
        }
    },

    onChangeCurrentModuleProps: function(currentModuleProps){
        this.getView().lookupReference('commonProperties').setSource(currentModuleProps.common);
        this.getView().lookupReference('specificProperties').setSource(currentModuleProps.specific);
        this.getView().lookupReference('telemetrySelect').setSource(currentModuleProps.telemetrySubscriptions);
    },

    // Вызывается при нажатии кнопки Save в диалоге SaveAs (SaveAsSchemaDialog)
    onClickSaveAsSchema: function(){
        var model = this.getView().getViewModel();
        var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];
        var svgpanelcontroller = svgpanelmodel.getView().controller;
        svgpanelcontroller.SaveAsSchema(model.get('newSchemaName'), model.get('newSchemaVersion'));
    },

    // Вызывается при нажатии кнопки Save в диалоге SaveAs (SaveAsSchemaDialog)
    onClickDeleteModule: function(){
        var model = this.getView().getViewModel();
        var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];

        // Очисти окно свойств инстанса
        this.getView().lookupReference('commonProperties').setSource(null);
        this.getView().lookupReference('specificProperties').setSource(null);

        // Удалим инстанс со схемы
        svgpanelmodel.get('selectedCell').remove();
        svgpanelmodel.set('selectedCell', null);

        // Удалим параметры инстанса из схемы
        delete svgpanelmodel.get('currentSchema').get('modulesParams')[model.get('nameOfSelectedInstance')];

        // Установим заголовок в коне свйоств, это заблокирует кнопку удаления
        model.set('nameOfSelectedInstance', 'Properties');

        // Пометим схему как измененную
        svgpanelmodel.set('schemaChanged', true);
    }


});
