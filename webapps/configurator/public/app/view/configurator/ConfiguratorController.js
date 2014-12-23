/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.ConfiguratorController', {
    extend: 'Ext.app.ViewController',

    alias: 'controller.configurator',

    init: function () {
        var model = this.getView().getViewModel();

        // Подписываемся на факт изменения текущей схемы
        model.bind('{currentModuleProps}', this.onChangeCurrentModuleProps);

        // При обновлении параметров модуля, пометить схему как измененную.
        this.getView().lookupReference('commonProperties').getStore().on('update', this.markSchemaAsChanged, this);
        this.getView().lookupReference('specificProperties').getStore().on('update', this.markSchemaAsChanged, this);
        this.getView().lookupReference('telemetrySelect').getStore().on('update', this.onTelemetrySubscribeChanged, this);

        model.bind('{typeSelectedLink}', function (newVal) {
            var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];
            var selectedLink = svgpanelmodel.get('selectedLink');
            if (selectedLink && selectedLink.model.attributes.mode !== newVal) {
                selectedLink.model.attributes.mode = newVal;

                switch (newVal) {
                    case 'queue':
                        selectedLink.model.attributes.attrs[".connection"] = { stroke: 'red' };
                        selectedLink.model.attributes.attrs[".marker-target"].fill = 'red';
                        break;

                    case 'memory':
                        selectedLink.model.attributes.attrs[".connection"] = { stroke: 'blue' };
                        selectedLink.model.attributes.attrs[".marker-target"].fill = 'blue';
                        break;
                }
                selectedLink.update();

                // Пометим схему как измененную
                svgpanelmodel.set('schemaChanged', true);
            }
        });

    },

    onTelemetrySubscribeChanged: function (store, record, operation, modifiedFieldNames, eOpts) {
        if (operation == 'edit') {
            //var model = this.getView().getViewModel();
            //model.children['rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1'].set('schemaChanged', true);

            var cmd = 'unsubscribe';
            if (record.data.value) {
                cmd = 'subscribe';
            }
            var telemetrySelectPanel = this.getView().lookupReference('telemetrySelect');
            var svgController = this.getView().lookupReference('SvgPanel').getController();
            svgController.Subscribe2Telemetry(cmd, telemetrySelectPanel.ownerCt.ownerCt.ownerCt.title, record.data.name);
        }
        this.markSchemaAsChanged(store, record, operation, null, null);
    },

    markSchemaAsChanged: function (store, record, operation, modifiedFieldNames, eOpts) {
        if (operation == 'edit') {
            var model = this.getView().getViewModel();
            model.children['rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1'].set('schemaChanged', true);
        }
    },

    onChangeCurrentModuleProps: function (currentModuleProps) {
        this.getView().lookupReference('commonProperties').setSource(currentModuleProps.common);
        this.getView().lookupReference('specificProperties').setSource(currentModuleProps.specific);
        this.getView().lookupReference('telemetrySelect').setSource(currentModuleProps.telemetrySubscriptions);
    },

    // Вызывается при нажатии кнопки Save в диалоге SaveAs (SaveAsSchemaDialog)
    onClickSaveAsSchema: function () {
        var model = this.getView().getViewModel();
        var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];
        var svgpanelcontroller = svgpanelmodel.getView().controller;
        svgpanelcontroller.SaveAsSchema(model.get('newSchemaName'), model.get('newSchemaVersion'));
    },

    // Вызывается при нажатии кнопки Save в диалоге SaveAs (SaveAsSchemaDialog)
    onClickDeleteModule: function () {
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
    },


    onImportSchema: function () {
        var formPanel = this.getView().lookupReference('importdialog');

        var model = this.getView().getViewModel();
        var svgpanelmodel = model.children["rtconfigurator-view-configurator-svgpanel-svgpanelmodel-1"];
        var svgpanelcontroller = svgpanelmodel.getView().controller;

        var reader = new FileReader();
        reader.onload = function (e) {
            var importedSchema = Ext.JSON.decode(e.target.result, true);
            if (importedSchema == null) {
                Ext.window.MessageBox.show({
                    title: 'Error',
                    msg: 'Invalid file format',
                    icon: Ext.window.MessageBox.ERROR,
                    buttons: Ext.Msg.OK
                });
                return;
            }
            svgpanelcontroller.AddNewSchema(importedSchema);

            formPanel.close();
        };
        var file = formPanel.getForm().findField('file').extractFileInput().files[0];
        reader.readAsText(file);
        formPanel.close();
    }


});
