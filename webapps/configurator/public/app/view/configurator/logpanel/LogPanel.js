/**
 * Created by vrubel on 24.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogPanel', {
    extend: 'Ext.tab.Panel',
    alias: 'widget.logpanel',
    reference: 'LogPanel',

    listeners: {
        render: function (th, eOpts) {
            var svgPanelController = th.ownerCt.ownerCt.ownerCt.lookupReference('SvgPanel').getController()
            svgPanelController.initWebSockets(th);
        }
    },

    title: 'logs',

    requires: [
        'RtConfigurator.view.configurator.logpanel.LogModel',
        'RtConfigurator.view.configurator.logpanel.LogController'
    ],

    viewModel: {
        type: 'logpanel'
    },
    controller: 'logpanel',
    bodyPadding: 0,

    tabPosition: 'bottom',
    height: 300,
//    border: true,
    bodyStyle: {
        opacity: 0.8
    },
    header: {
        xtype: 'header',
        title: 'logs',
        items: [
            {
                xtype: 'panel',
                bodyPadding: 3,
                margin: 3,
                bind: {
                    bodyStyle: {
                        background: '{telemetryLabelBackground}'
                    }
                },
                bodyStyle: {
                    'text-align': 'center'
                },
                html: 'Telemetry',
                listeners: {
                    render: {
                        fn: function (th, eOpts) {
                            // после рендеринга установим всплывающую подсказку
                            var tip = Ext.create('Ext.tip.ToolTip', {
                                target: th.el,
                                html: 'Telemetry WebSocket status'
                            });
                        }
                    }
                }
            },
            {
                xtype: 'panel',
                bodyPadding: 3,
                margin: 3,
                bind: {
                    bodyStyle: {
                        background: '{logLabelBackground}'
                    }
                },
                bodyStyle: {
                    'text-align': 'center'
                },
                html: 'Log',
                listeners: {
                    render: {
                        fn: function (th, eOpts) {
                            // после рендеринга установим всплывающую подсказку
                            var tip = Ext.create('Ext.tip.ToolTip', {
                                target: th.el,
                                html: 'Log WebSocket status'
                            });
                        }
                    }
                }
            }
        ]},

    items:[
        {
            title:'ssss',
            html: 'ddddd'
        }
    ]
});

