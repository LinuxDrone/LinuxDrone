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

        model.set('graph', new joint.dia.Graph);

        model.set('paper', new joint.dia.Paper({
            el: this.el.dom,
            model: model.get('graph'),
            gridSize: 1,
            width: "100%",
            height: "99%"
        }));

    }
});
