/**
 * Created by vrubel on 29.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogController', {
    extend: 'Ext.app.ViewController',

    alias: 'controller.logpanel',

    init: function () {
        var view = this.getView();
        var model = view.getViewModel();

        model.bind('{expanded}', function (expanded) {
            /*
            var logPanel = this.getView();
            if (!logPanel.rendered) return;

            var newY = logPanel.getPosition()[1] + 27;
            logPanel.origY = logPanel.getPosition()[1];
            if(expanded){
                newY -= logPanel.getHeight();
            }else{
                newY += logPanel.getHeight() - 54;
            }

            logPanel.animate({
                to: {
                    y: newY
                },
                duration: 500
            });
            */
        });

/*
        var labelTelemetryStatus = view.lookupReference('LabelTelemetryStatus');

        console.log(labelTelemetryStatus.rendered);
*/

    }
});
