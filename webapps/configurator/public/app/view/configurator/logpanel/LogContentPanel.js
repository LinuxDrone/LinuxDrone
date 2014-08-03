/**
 * Created by vrubel on 03.08.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogContentPanel', {
    extend: 'Ext.grid.Panel',
    autoScroll: true,
    hideHeaders: true,
    store: 'StoreMetaModules',
    columns: [
        { text: 'Module', dataIndex: 'name', padding: 5, width: '80%'},

    ],
    region: 'west',
    collapsible: true,
    width: 250,
    split: true
});