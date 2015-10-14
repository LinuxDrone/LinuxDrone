/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.SettingsListPanel', {
    extend: 'Ext.grid.Panel',
    alias: 'widget.settingslist',

    title: 'Settings List',
    hideHeaders: true,
    store: 'MetamodulesStore',
    columns: [
        { text: 'Module', dataIndex: 'name', padding: 5, width: '80%'},
        {
            xtype: 'actioncolumn',
            align: 'right',
            width: 40,

            items: [
                {
                    icon: 'images/information.png',
                    tooltip: 'About module',
                    handler: function (grid, rowIndex, colIndex) {
                        var rec = grid.getStore().getAt(rowIndex);
                        Ext.Msg.alert('Module description', rec.get('description').en);
                    },
                    getClass: function (v, metadata, r, rowIndex, colIndex, store) {
                        if (r.data.description == undefined) {
                            return " x-hidden";
                        }
                    }
                },
                {
                    icon: 'images/add.png',
                    tooltip: 'Add to scheme',
                    handler: function (grid, rowIndex, colIndex) {
                        var rec = grid.getStore().getAt(rowIndex);
                        grid.ownerCt.ownerCt.layout.centerRegion.controller.AddModule2Scheme(rec.data);
                    }
                }
            ]
        }
    ],
    region: 'west',
    collapsible: true,
    width: 250,
    split: true

/*
    viewModel: {
        type: 'settingslist'
    },
    controller: 'settingslist',

    layout: 'border',
    reference: 'settingsList',

    title: 'Settings',

    items: [
        {
            xtype: 'grid',
            autoScroll: true,
            title: 'Modules',
            hideHeaders: true,
            store: 'MetamodulesStore',
            columns: [
                { text: 'Module', dataIndex: 'name', padding: 5, width: '80%'},
                {
                    xtype: 'actioncolumn',
                    align: 'right',
                    width: 40,

                    items: [
                        {
                            icon: 'images/information.png',
                            tooltip: 'About module',
                            handler: function (grid, rowIndex, colIndex) {
                                var rec = grid.getStore().getAt(rowIndex);
                                Ext.Msg.alert('Module description', rec.get('description').en);
                            },
                            getClass: function (v, metadata, r, rowIndex, colIndex, store) {
                                if (r.data.description == undefined) {
                                    return " x-hidden";
                                }
                            }
                        },
                        {
                            icon: 'images/add.png',
                            tooltip: 'Add to scheme',
                            handler: function (grid, rowIndex, colIndex) {
                                var rec = grid.getStore().getAt(rowIndex);
                                grid.ownerCt.ownerCt.layout.centerRegion.controller.AddModule2Scheme(rec.data);
                            }
                        }
                    ]
                }
            ],
            region: 'west',
            collapsible: true,
            width: 250,
            split: true
        }
        */
    //]
});