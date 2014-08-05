/**
 * Created by vrubel on 03.08.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogContentPanel', {
    extend: 'Ext.grid.Panel',
    alias: 'widget.logcontentpanel',
    //autoScroll: true,
    hideHeaders: true,
    columnLines: false,
    rowLines: false,
    viewConfig: {
        stripeRows: false,
        trackOver: false
        //selectedItemCls : 'x-grid-item'
    },
    //overCls: 'x-grid-item',
    scroll: 'vertical',

    bodyStyle: {
        //background:'black',
        //opacity:0.8
    },

    store: Ext.create('Ext.data.Store', {
        model: 'RtConfigurator.model.LogRecord'/*,
        data: [
            {log: 'Spencer'},
            {log: 'Tommy'}
        ]
        */
    }),

    columns: [
        { text: 'log', dataIndex: 'log', padding: 0, width: '100%'}
    ]
});