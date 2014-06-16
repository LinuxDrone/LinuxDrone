/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */



Ext.define('RtConfigurator.view.svg.SvgPanel', {
    extend : 'Ext.Panel',
    title: 'SVG',
    padding: 5,
    alias: 'widget.svgpanel',

    graph: undefined,

    paper: undefined,

    afterRender: function() {
console.log("afterRender"); 
        if(this.graph != undefined){
            return;
        }

        this.graph = new joint.dia.Graph;

        this.paper = new joint.dia.Paper({
            el: this.el.dom,
            model: this.graph,
            gridSize: 1
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

/*
Ext.define('RtConfigurator.view.svg.SvgPanel', {
    extend: 'Ext.Panel',

    alias: 'widget.svgpanel',

    graph: new joint.dia.Graph,

    paper: new joint.dia.Paper({
    el: $('#dddf'),
    gridSize: 20,
    model: this.graph,
    defaultLink: new joint.dia.Link({
        attrs: {
            '.marker-target': {fill: 'red', d: 'M 10 0 L 0 5 L 10 10 z' },
            '.connection': {stroke: 'red', 'stroke-width': "2"}
        }
    }),
    validateConnection: function (cellViewS, magnetS, cellViewT, magnetT) {
        //return true;
        // Prevent linking from input ports.
        if (magnetS && magnetS.attributes.fill.value === viewModels.Editor.inPortsFillColor) return false;
        // Prevent linking from output ports to input ports within one element.
        if (cellViewS === cellViewT) return false;
        // Prevent linking to input ports.
        return magnetT && magnetT.attributes.fill.value === viewModels.Editor.inPortsFillColor;
    }
    /*
     ,
     validateMagnet: function (cellView, magnet) {
     // Note that this is the default behaviour. Just showing it here for reference.
     // Disable linking interaction for magnets marked as passive (see below `.inPorts circle`).
     return magnet.getAttribute('magnet') !== 'passive';
     }

}),

    title: 'SVG',
    padding: 5,
    html: 'My Company - SVG'
});
*/