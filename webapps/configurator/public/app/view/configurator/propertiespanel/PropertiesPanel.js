/**
 * Created by vrubel on 18.07.14.
 */
Ext.define('RtConfigurator.view.configurator.propertiespanel.PropertiesPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.propertiespanel',

    bind:{
        title: '{nameOfSelectedInstance}'
    },
    collapsible: true,
    split: true,
    width: 300,
    layout: {
        type: 'accordion',
        animate: true,
        activeOnTop: true
    },
    items: [
        {
            title: 'Common Properties',
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'commonProperties'
                }
            ]
        },
        {
            title: 'Specific Properties',
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'specificProperties'
                }
            ]
        },
        {
            title: 'Telemetry',
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'telemetrySelect'
                }
            ]
        }
    ],
    bbar: [
        {
            text: 'Delete',
            handler: 'onClickDeleteModule',
            tooltip: 'Delete module instance from schema',
            bind:{
                disabled: '{disableDeleteModule}'
            }
        }
    ]
});