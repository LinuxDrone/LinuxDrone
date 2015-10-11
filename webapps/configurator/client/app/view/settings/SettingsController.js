/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.SettingsController', {
    extend: 'Ext.app.ViewController',

    alias: 'controller.settings',

    init: function () {
        /*
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
        */
    }


});
