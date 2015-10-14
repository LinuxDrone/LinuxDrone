/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.SettingsPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.settings',

    requires: [
        //'RtConfigurator.view.configurator.svgpanel.SvgPanel',
        //'RtConfigurator.view.configurator.propertiespanel.PropertiesPanel'
    ],

    viewModel: {
        type: 'settings'
    },
    controller: 'settings',

    layout: 'border',
    reference: 'settingsTab',

    title: 'Settings',

    items: [
        {
            xtype: 'settingslist'
        }
    ]
});