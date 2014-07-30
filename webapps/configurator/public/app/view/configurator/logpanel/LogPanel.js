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
    bodyPadding: 5,
    width: 550,

    viewModel: {
        type: 'logpanel'
    },
    controller: 'logpanel',
    bodyPadding: 0,

    tabPosition: 'bottom',
    height: 200,
    border: true,


    items: [
        {
            title: 'Bar',
            html: 'A simple tab'
        },
        {
            title: 'Foo',
            html: 'A simple tab 2'
        }
    ],

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
        }
    ]

});

