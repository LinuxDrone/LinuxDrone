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
    },
    cls: 'custom-grid',
    scroll: 'vertical',

    store: Ext.create('Ext.data.Store', {
        model: 'RtConfigurator.model.LogRecord'
    }),

    columns: [
        { text: 'log', dataIndex: 'log', padding: 0, width: '100%'}
    ]
});