/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */



Ext.define('RtConfigurator.view.svg.SvgPanel', {
    extend: 'Ext.Component',
    alias: 'widget.svgpanel',

    requires: [
        'RtConfigurator.view.svg.SvgPanelController'
    ],

    viewModel: {
        type: 'svg'
    },
    controller: 'svg',

    graph: undefined,

    paper: undefined,

    afterRender: function() {

        if(this.graph != undefined){
            return;
        }

        this.graph = new joint.dia.Graph;

        this.paper = new joint.dia.Paper({
            el: this.el.dom,
            model: this.graph,
            gridSize: 1,
            width: "100%",
            height: "99%"
        });

        var rect = new joint.shapes.basic.Rect({
            position: { x: 100, y: 30 },
            size: { width: 100, height: 30 },
            attrs: { rect: { fill: 'blue' }, text: { text: 'my box', fill: 'white' } }
        });

        var rect2 = rect.clone();
        rect2.translate(300);

        var link = new joint.dia.Link({
            source: { id: rect.id },
            target: { id: rect2.id }
        });

        this.graph.addCells([rect, rect2, link]);
    }

});
