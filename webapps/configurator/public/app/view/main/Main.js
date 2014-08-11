/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */
Ext.define('RtConfigurator.view.main.Main', {
    extend: 'Ext.container.Container',

    requires: [
        //'RtConfigurator.view.configurator.ConfiguratorPanel'
    ],

    xtype: 'app-main',

    controller: 'main',
    viewModel: {
        type: 'main'
    },

    layout: {
        type: 'border'
    },

    items: [
        {
            region: 'center',
            xtype: 'tabpanel',
            items: [
                /*{
                    title: 'Tab 1',
                    html: '<h2>Content appropriate for the current navigation.</h2>'
                },*/
                {
                    xtype: 'configurator',
                    title: 'Configuration'
                }
            ]
        }
    ]
});
