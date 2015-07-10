Ext.define('RtConfigurator.view.configurator.dialogs.LoginDialog', {
    extend: 'Ext.form.Panel',
    floating: true,
    modal: true,
    title: 'Addresses',
    bodyPadding: 5,
    width: 350,

    requires: [
        'RtConfigurator.view.configurator.ConfiguratorPanel'
    ],

    // Fields will be arranged vertically, stretched to full width
    layout: 'anchor',
    defaults: {
        anchor: '100%'
    },

    // The fields
    defaultType: 'textfield',
    items: [
        {
            fieldLabel: 'Server',
            name: 'server',
            allowBlank: false,
            value: 'localhost:4000'
        },
        {
            fieldLabel: 'Telemetry',
            name: 'telemetry',
            allowBlank: true,
            value: 'localhost:7681'
        }
    ],

    // Reset and Submit buttons
    buttons: [
        {
            text: 'Connect',
            formBind: true, //only enabled once the form is valid
            disabled: true,

            handler: function () {
                var form = this.up('form').getForm();

                var values = form.getFieldValues();

                //console.log(values.server);
                //console.log(values.telemetry);

                // Добавим вкладку конфигуратора в главный лайаут приложения
                this.ownerCt.ownerCt.floatParent.items.items[0].add(new RtConfigurator.view.configurator.ConfiguratorPanel(
                    {
                        url_server: values.server,
                        url_telemetry: values.telemetry
                    }
                ));

                // Найдем хранилище и иницируем загрузку в него определения модулей
                var storeMetaModules = Ext.StoreMgr.lookup("StoreMetaModules");
                storeMetaModules.getProxy().setUrl( 'http://' + values.server + '/metamodules' );
                storeMetaModules.load();

                this.up('form').close();
            }
        }
    ]
});

