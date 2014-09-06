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
            th.logPanel.setWidth(th.getWidth());

            var modelLogPanel = th.logPanel.getViewModel();
            var expanded = modelLogPanel.get('expanded');

            if (expanded) {
                th.logPanel.showAt(th.getPosition()[0], th.getPosition()[1] + th.getHeight() - th.logPanel.getHeight()+27);
            } else {
                th.logPanel.showAt(th.getPosition()[0], th.getPosition()[1] + th.getHeight());
            }
        }
    },

    autoScroll: true,
    overflowX: 'auto',
    margin: '0 0 30 0',

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
        { text: '+', handler: 'onClickZoomIn', tooltip: 'Zoom In' },
        {
            xtype: 'panel',
            bind: {
                html: '{XenoCPU}'
            },
            listeners: {
                render: {
                    fn: function( th, eOpts ){
                        // после рендеринга установим всплывающую подсказку
                        var tip = Ext.create('Ext.tip.ToolTip', {
                            target: th.el,
                            html: 'CPU Load by realtime threads'
                        });
                    }
                }
            }
        }
    ],

    logPanel: Ext.create('RtConfigurator.view.configurator.logpanel.LogPanel')

});