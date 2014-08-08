/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.configurator.svgpanel.SvgPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.svgpanel',
    reference: 'SvgPanel',

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
        afterlayout: function (th, options) {

            /*
            var logPanel = th.lookupReference('LogPanel');
            if(!logPanel) return;
            
            logPanel.setWidth(th.getWidth());

            var modelLogPanel = logPanel.getViewModel();
            var expanded = modelLogPanel.get('expanded');

            if (expanded) {
                logPanel.showAt(th.getPosition()[0], th.getPosition()[1] + th.getHeight() - logPanel.getHeight());
            } else {
                logPanel.showAt(th.getPosition()[0], th.getPosition()[1] + th.getHeight() - 27 -200);
            }*/
        },
        render: function(th, eOpts ){
          //  th.getController().initWebSockets();
        }
    },

    layout: 'fit',
    items: [
        {xtype: 'svg'}
    ],
    tbar: {
        bind: {
            disabled: '{started}'
        },
        items: [
            {
                xtype: 'combo',
                editable: false,
                fieldLabel: 'Schema',
                labelWidth: 50,
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
                labelWidth: 50,
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
                tooltip: 'Save as current configuration',
                bind: {
                    //disabled: '{!schemaChanged}'
                }
            },
            '-',
            {
                xtype: 'button',
                text: 'Delete',
                handler: 'onClickDeleteSchema',
                tooltip: 'Delete current configuration',
                bind: {
                    //disabled: '{!schemaChanged}'
                }
            },
            '-',
            {
                xtype: 'panel',
                bind: {
                    html: '{exportLink}'
                }
            },
            {
                xtype: 'button',
                text: 'Import',
                handler: 'onClickOpenImportSchemaDialog',
                tooltip: 'Import Schema from file'
            }
        ]
    },

    rbar: [
        { text: '<div style="color: green">&#9654;</div>', handler: 'RunConfig', tooltip: 'Start',
            bind: {
                hidden: '{started}'
            }},
        { text: '<div style="color: red">&#9724;</div>', handler: 'StopConfig', tooltip: 'Stop',
            bind: {
                hidden: '{!started}'
            }},
        '',
        '',
        { text: '-', handler: 'onClickZoomOut', tooltip: 'Zoom Out'},
        { text: '+', handler: 'onClickZoomIn', tooltip: 'Zoom In' }
    ]//,

    //logPanel: Ext.create('RtConfigurator.view.configurator.logpanel.LogPanel')

})
;