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
            width: "100%",
            height: "99%",
            defaultLink: new joint.dia.Link({
                attrs: {
                    '.marker-target': {fill: 'red', d: 'M 10 0 L 0 5 L 10 10 z' },
                    '.connection': {stroke: 'red', 'stroke-width': "2"}
                }
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
                // Вызов контекстного меню по правой кнопке мыши
                if (evt.button == 2) {
                    //console.log("press " + x + " " + y);
                    var paperPosition = $("#paper").position();
                    $("#moduleContextMenu").css({
                        display: "block",
                        left: x + paperPosition.left,
                        top: y + paperPosition.top
                    });
                }
            }
            else {
                if (cellView.model.attributes.type == "link") {
                    if (evt.button == 2) {
                        var paperPosition = $("#paper").position();
                        $("#linkContextMenu").css({
                            display: "block",
                            left: x + paperPosition.left,
                            top: y + paperPosition.top
                        });
                    }
                    res.selectedLink(cellView);
                }
            }
        });

        // Подпишемся на изменения графа
        graph.on('change', function () {
            model.set('schemaChanged', true);
        });

    }
});
