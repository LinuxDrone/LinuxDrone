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
    layout:'fit',

    items:[
        {
            layout: {
                type: 'accordion',
                animate: true,
                activeOnTop: true
            },
            bind: {
                hidden: '{hideInstanceProperties}'
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
                    bind: {
                        disabled: '{disableDeleteModule}'
                    }
                }
            ]
        },
        {
            bodyPadding: 5,
            reference: 'panelLinkProperties',
            bind: {
                hidden: '{hideLinkProperties}'
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
    ]


});