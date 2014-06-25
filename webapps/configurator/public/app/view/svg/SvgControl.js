/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */
Ext.define('RtConfigurator.view.svg.SvgControl', {
    extend: 'Ext.Component',
    alias: 'widget.svg',

    afterRender: function() {

        var model = this.ownerCt.getViewModel();
        if(model.graph != undefined){
            return;
        }

        model.graph = new joint.dia.Graph;

        model.paper = new joint.dia.Paper({
            el: this.el.dom,
            model: model.graph,
            gridSize: 1,
            width: "100%",
            height: "99%"
        });
/*
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

        model.graph.addCells([rect, rect2, link]);
        */
    }
});
