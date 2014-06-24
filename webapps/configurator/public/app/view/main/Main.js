/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */
Ext.define('RtConfigurator.view.main.Main', {
    extend: 'Ext.container.Container',

    requires: [
        'RtConfigurator.view.svg.SvgPanel'
    ],

    xtype: 'app-main',
    
    controller: 'main',
    viewModel: {
        type: 'main'
    },

    layout: {
        type: 'border'
    },

    items: [{
        xtype: 'grid',
        /*bind: {
            title: '{name}'
        },*/
        title: 'Modules',
        hideHeaders :true,
        store: 'StoreMetaModules',
        columns: [
            { text: 'Module',  dataIndex: 'name', padding:5, width:'80%'},
            {
                xtype:'actioncolumn',
                align:'right',
                width:40,

                items: [{
                    icon: 'images/information.png',
                    tooltip: 'About module',
                    handler: function(grid, rowIndex, colIndex) {
                        var rec = grid.getStore().getAt(rowIndex);
                        Ext.Msg.alert('Module description', rec.get('description').en);
                    },
                    getClass: function(v, metadata, r, rowIndex, colIndex, store) {
                        if(r.data.description == undefined) {
                            return " x-hidden";
                        }
                    }
                },{
                    icon: 'images/add.png',
                    tooltip: 'Add to scheme',
                    handler: function(grid, rowIndex, colIndex) {
                        var rec = grid.getStore().getAt(rowIndex);
                        grid.ownerCt.ownerCt.controller.onAddModule2Scheme(rec.data);
                    }
                }]
            }
        ],
        region: 'west',
        collapsible: true,
        width: 250,
        split: true/*,
        tbar: [{
            text: 'Button',
            handler: 'onClickButton'
        }]*/
    },{
        region: 'center',
        xtype: 'tabpanel',
        items:[{
            title: 'Tab 1',
            html: '<h2>Content appropriate for the current navigation.</h2>'
        },
            {
                layout: 'border',
                xtype: 'panel',
                reference: 'svgTab',
                title: 'Configuration',
                items:[{
                    region: 'east',
                    title: 'Properties',
                    collapsible: true,
                    width: 250,
                    split: true,
                    html: '<h2>properties will be here</h2>'
                },{
                    region: 'center',
                    xtype: 'svgpanel',
                    reference: 'svgPanel'
                }]
            }
        ]
    }]
});
