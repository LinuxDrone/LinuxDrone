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
            title: 'common',
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'commonProperties'
                }
            ]
        },
        {
            title: 'specific',
            items: [
                {
                    xtype: 'propertygrid',
                    reference: 'specificProperties'
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