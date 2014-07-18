/**
 * Created by vrubel on 18.07.14.
 */
Ext.define('RtConfigurator.view.configurator.propertiespanel.PropertiesPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.propertiespanel',

    title: 'Properties',
    header: false,
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
                    source: {
                        "Task Priority": 66,
                        "Task Period": 150,
                        "Transfer task period": 20
                    }
                }
            ]
        },
        {
            title: 'specific',
            items: [
                {
                    xtype: 'propertygrid',
                    source: {
                        "Pru Device" : 1,
                        "Pru Binary" : "/root/PwmOut.bin"
                    }
                }
            ]
        }
    ]
});