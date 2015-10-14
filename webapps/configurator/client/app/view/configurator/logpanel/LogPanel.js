/**
 * Created by vrubel on 24.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogPanel', {
    extend: 'Ext.tab.Panel',

    requires: [
        'RtConfigurator.view.configurator.logpanel.LogModel',
        'RtConfigurator.view.configurator.logpanel.LogController'
    ],

    floating: true,
    autoShow: false,
    layout: 'fit',
    //bodyPadding: 5,
    width: 550,

    viewModel: {
        type: 'logpanel'
    },
    controller: 'logpanel',
    bodyPadding: 0,
    resizable: true,
    resizeHandles: 'n',

    tabPosition: 'bottom',
    height: 300,
    border: true,
    bodyStyle:{
        opacity: 0.8
    },
    rbar: [
        {
            xtype: 'tool',
            type: 'up',
            bind: {
                hidden: '{expanded}'
            },
            callback: function (tbar) {
                var logPanel = tbar.ownerCt;
                logPanel.getViewModel().set('expanded', true);
            }
        },
        {
            xtype: 'tool',
            type: 'down',
            bind: {
                hidden: '{!expanded}'
            },
            callback: function (tbar) {
                var logPanel = tbar.ownerCt;
                logPanel.getViewModel().set('expanded', false);
            }
        },{
            xtype:'panel',
            bind: {
                bodyStyle: {
                    background : '{telemetryLabelBackground}'
                }
            },
            bodyStyle:{
                'text-align': 'center'
            },
            html: 'T',
            listeners: {
                render: {
                    fn: function( th, eOpts ){
                        // после рендеринга установим всплывающую подсказку
                        var tip = Ext.create('Ext.tip.ToolTip', {
                            target: th.el,
                            html: 'Telemetry WebSocket status'
                        });
                    }
                }
            }
        },{
            xtype:'panel',
            bind: {
                bodyStyle: {
                    background : '{logLabelBackground}'
                }
            },
            bodyStyle:{
                'text-align': 'center'
            },
            html: 'L',
            listeners: {
                render: {
                    fn: function( th, eOpts ){
                        // после рендеринга установим всплывающую подсказку
                        var tip = Ext.create('Ext.tip.ToolTip', {
                            target: th.el,
                            html: 'Log WebSocket status'
                        });
                    }
                }
            }
        }
    ]
});

