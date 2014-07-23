/**
 * Created by vrubel on 18.07.14.
 */
Ext.define('RtConfigurator.view.configurator.propertiespanel.PropertiesPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.propertiespanel',

    bind: {
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
            bind: {
                //hidden: '{hideInstanceProperties}'
            },
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'commonProperties'
                }
            ]
        },
        {
            title: 'Specific Properties',
            bind: {
                //hidden: '{hideInstanceProperties}'
            },
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'specificProperties'
                }
            ]
        },
        {
            title: 'Telemetry',
            bind: {
                //hidden: '{hideInstanceProperties}'
            },
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'telemetrySelect'
                }
            ]
        },
        {
            title: 'Link Properties',
            reference: 'panelLinkProperties',
            bind: {
                //hidden: '{hideLinkProperties}'
            },
            items: [
                {
                    xtype: 'combo',
                    fieldLabel: 'Link Type',
                    store: {
                        fields: ['abbr', 'name'],
                        data: [
                            {"abbr": "queue", "name": "queue"},
                            {"abbr": "memory", "name": "memory"}
                        ]
                    },
                    queryMode: 'local',
                    displayField: 'name',
                    valueField: 'abbr',
                    editable: false,
                    bind: {
                        value: '{typeSelectedLink}'
                    },
                    listeners: {
                        //select: 'onSelectSchema'
                    }
                }
            ]
        }
    ],
    bbar: [
        {
            text: 'Delete',
            handler: 'onClickDeleteModule',
            tooltip: 'Delete module instance from schema',
            bind: {
                disabled: '{disableDeleteModule}'
            }
        }
    ]
});