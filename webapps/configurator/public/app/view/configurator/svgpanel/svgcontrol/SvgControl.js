/**
 * This class is the main view for the application. It is specified in app.js as the
 * "autoCreateViewport" property. That setting automatically applies the "viewport"
 * plugin to promote that instance of this class to the body element.
 *
 * TODO - Replace this content of this view to suite the needs of your application.
 */
Ext.define('RtConfigurator.view.configurator.svgpanel.svgcontrol.SvgControl', {
    extend: 'Ext.Component',
    alias: 'widget.svg',

    afterRender: function() {

        var model = this.ownerCt.getViewModel();
        if(model.graph != undefined){
            return;
        }

        var graph = new joint.dia.Graph;

        graph.on('add', function (cell) {
            if (cell.attributes.type == "link") {
                // При создании нового линка, ему по умолчанию устанавливается тип и цвет.
                cell.attributes["mode"] = "queue";

                var moduleType = model.getView().getController().GetModuleTypeByGraphId(cell.attributes.source.id);
                var storeMetamodules = Ext.data.StoreManager.lookup('StoreMetaModules');
                var moduleDef = storeMetamodules.findRecord('name', moduleType);

                var group = _.find(moduleDef.get('outputs'), function (group) {
                    return cell.attributes.source.port in group.Schema.properties;
                });

                cell.attributes["nameOutGroup"] = group.name;
                cell.attributes["portType"] = group.Schema.properties[cell.attributes.source.port].type;
            }
        });


        model.set('graph', graph);

        var paper = new joint.dia.Paper({
            el: this.el.dom,
            model: graph,
            gridSize: 1,
            width: "2880px",
            height: "1620px",
            defaultLink: new joint.dia.Link({
                attrs: {
                    '.marker-target': {fill: 'red', d: 'M 10 0 L 0 5 L 10 10 z' },
                    '.connection': {stroke: 'red', 'stroke-width': "2"}//, 'stroke-dasharray':"5, 5"
                },
                //manhattan: true,
                toolMarkup: [
                    '<g class="link-tool">',
                    '<g class="tool-remove" event="remove">',
                    '<circle r="11" />',
                    '<path transform="scale(.8) translate(-16, -16)" d="M24.778,21.419 19.276,15.917 24.777,10.415 21.949,7.585 16.447,13.087 10.945,7.585 8.117,10.415 13.618,15.917 8.116,21.419 10.946,24.248 16.447,18.746 21.948,24.248z"/>',
                    '<title>Remove link.</title>',
                    '</g>',
                    '<g event="link:options">',
                    '<circle r="11" transform="translate(25)"/>',
                    '<path fill="white" transform="scale(.55) translate(29, -16)" d="M31.229,17.736c0.064-0.571,0.104-1.148,0.104-1.736s-0.04-1.166-0.104-1.737l-4.377-1.557c-0.218-0.716-0.504-1.401-0.851-2.05l1.993-4.192c-0.725-0.91-1.549-1.734-2.458-2.459l-4.193,1.994c-0.647-0.347-1.334-0.632-2.049-0.849l-1.558-4.378C17.165,0.708,16.588,0.667,16,0.667s-1.166,0.041-1.737,0.105L12.707,5.15c-0.716,0.217-1.401,0.502-2.05,0.849L6.464,4.005C5.554,4.73,4.73,5.554,4.005,6.464l1.994,4.192c-0.347,0.648-0.632,1.334-0.849,2.05l-4.378,1.557C0.708,14.834,0.667,15.412,0.667,16s0.041,1.165,0.105,1.736l4.378,1.558c0.217,0.715,0.502,1.401,0.849,2.049l-1.994,4.193c0.725,0.909,1.549,1.733,2.459,2.458l4.192-1.993c0.648,0.347,1.334,0.633,2.05,0.851l1.557,4.377c0.571,0.064,1.148,0.104,1.737,0.104c0.588,0,1.165-0.04,1.736-0.104l1.558-4.377c0.715-0.218,1.399-0.504,2.049-0.851l4.193,1.993c0.909-0.725,1.733-1.549,2.458-2.458l-1.993-4.193c0.347-0.647,0.633-1.334,0.851-2.049L31.229,17.736zM16,20.871c-2.69,0-4.872-2.182-4.872-4.871c0-2.69,2.182-4.872,4.872-4.872c2.689,0,4.871,2.182,4.871,4.872C20.871,18.689,18.689,20.871,16,20.871z"/>',
                    '<title>Link options.</title>',
                    '</g>',
                    '</g>'
                ].join('')
            })/*,

            validateConnection: function (cellViewS, magnetS, cellViewT, magnetT) {
                //return true;
                // Prevent linking from input ports.
                if (magnetS && magnetS.attributes.fill.value === viewModels.Editor.inPortsFillColor) return false;
                // Prevent linking from output ports to input ports within one element.
                if (cellViewS === cellViewT) return false;
                // Prevent linking to input ports.
                return magnetT && magnetT.attributes.fill.value === viewModels.Editor.inPortsFillColor;
            }
 */
        });

        model.set('paper', paper);

        paper.on('cell:pointerdown', function (cellView, evt, x, y) {
            if (cellView.model.attributes.type == "devs.Model") {
                model.set('selectedCell', cellView.model);
            }
        });

        // Обработчик выбора связи (клик по иконке свойства связи)
        paper.on('link:options', function(evt, linkView, x, y) {
            model.set('selectedLink', linkView);
        });

        // Подпишемся на изменения графа
        graph.on('change', function () {
            if(model.get('blockChangeSchema') === false){
                model.set('schemaChanged', true);
            }
        });

        paper.on('blank:pointerdown', function (evt, x, y) {
            if (evt.button == 0) {
                // Обнулим ссылку на выбранный инстанс
                model.set('selectedCell', null);

                // Обнулим ссылку на выбранную связь
                model.set('selectedLink', null);
            }
        });

        var svgPanelController = this.ownerCt.getController();
        svgPanelController.GetHostStatus();
    }
});
