/**
 * Created by vrubel on 22.06.14.
 */
Ext.define('RtConfigurator.view.svg.SvgPanel', {
    extend: 'Ext.Panel',
    alias: 'widget.svgpanel',

    requires: [
        'RtConfigurator.view.svg.SvgPanelController',
        'RtConfigurator.view.svg.SvgPanelModel',
        'RtConfigurator.view.svg.SvgControl'
    ],

    viewModel: {
        type: 'svg'
    },
    controller: 'svg',

    layout:'fit',
    items:[
        {xtype:'svg'}
    ],
    bbar: [
        { text: '-', handler: 'onClickZoomOut'},
        { text: '+', handler: 'onClickZoomIn' }
    ]
});