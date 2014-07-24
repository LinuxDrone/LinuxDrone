/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.configurator.svgpanel.SvgPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.svgpanel',

    requires: [
        'RtConfigurator.view.configurator.svgpanel.SvgPanelController',
        'RtConfigurator.view.configurator.svgpanel.SvgPanelModel',
        'RtConfigurator.view.configurator.svgpanel.svgcontrol.SvgControl'
    ],

    viewModel: {
        type: 'svgpanel'
    },
    controller: 'svgpanel',


    listeners: {
        afterrender: function(th, options) {
            th.logPanel.showAt(th.getPosition()[0] + 250, th.getPosition()[1] + th.getHeight() - 230);
        }
    },

    layout: 'fit',
    items: [
        {xtype: 'svg'}
    ],
    tbar: {
        xtype: 'toolbar',
        border: 5,
        style: {
            borderColor: 'red',
            borderStyle: 'solid'
        },
        items: [
            {
                xtype: 'combo',
                editable: false,
                fieldLabel: 'Schema',
                bind: {
                    store: '{listSchemasNames}',
                    value: '{currentSchemaName}'
                },
                displayField: 'name',
                listeners: {
                    select: 'onSelectSchema'
                },
                queryMode: 'local'
            },
            {
                xtype: 'combo',
                editable: false,
                fieldLabel: 'Version',
                bind: {
                    store: '{listSchemasVersions}',
                    value: '{currentSchemaVersion}'
                },
                displayField: 'version',
                valueField: 'version',
                listeners: {
                    select: 'onSelectVersion'
                },
                queryMode: 'local'
            },
            {
                xtype: 'button',
                text: 'Save',
                handler: 'onClickSaveSchema',
                tooltip: 'Save current configuration',
                bind: {
                    disabled: '{!schemaChanged}'
                }
            },
            {
                xtype: 'button',
                text: 'Save As',
                handler: 'onClickOpenSaveAsSchemaDialog',
                tooltip: 'Save as current configuration..',
                bind: {
                    //disabled: '{!schemaChanged}'
                }
            },
            {
                xtype: 'button',
                text: 'Delete',
                handler: 'onClickDeleteSchema',
                tooltip: 'Delete current configuration',
                bind: {
                    //disabled: '{!schemaChanged}'
                }
            }
        ]
    },
    buttons: [
        { text: '-', handler: 'onClickZoomOut', tooltip: 'Zoom Out'},
        { text: '+', handler: 'onClickZoomIn', tooltip: 'Zoom In' }
    ],

    logPanel: Ext.create('RtConfigurator.view.configurator.logpanel.LogPanel', {
        ownerCt: this.ownerCt
    })

});