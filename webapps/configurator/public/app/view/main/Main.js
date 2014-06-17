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
        'RtConfigurator.view.svg.SvgPanel',
    ],

    xtype: 'app-main',
    
    controller: 'main',
    viewModel: {
        type: 'main'
    },

    layout: {
        type: 'border'
    },

    items: [{
        xtype: 'grid',
        bind: {
            title: '{name}'
        },
        store: 'StoreMetaModules',
        columns: [
            { text: 'Module',  dataIndex: 'name' }
        ],
        region: 'west',
        width: 250,
        split: true,
        tbar: [{
            text: 'Button',
            handler: 'onClickButton'
        }]
    },{
        region: 'center',
        xtype: 'tabpanel',
        items:[{
            title: 'Tab 1',
            html: '<h2>Content appropriate for the current navigation.</h2>'
        },{
            title: 'List',
            xtype: 'multiselect',
            fieldLabel: 'Choose State',
            store: 'StoreMetaModules',
            //queryMode: 'local',
            displayField: 'name'//,
            //valueField: 'abbr',
        },{
            xtype: 'svgpanel',
            title: 'SVG'
        }]
    }]
});
