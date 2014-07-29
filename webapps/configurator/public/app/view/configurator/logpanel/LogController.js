/**
 * Created by vrubel on 29.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogController', {
    extend: 'Ext.app.ViewController',

    alias: 'controller.logpanel',

    init: function () {
        var model = this.getView().getViewModel();

        model.bind('{expanded}', function (newVal) {
            var logPanel = this.getView();
            if (!logPanel.rendered) return;

            var newY = logPanel.getPosition()[1] + 30;
            logPanel.origY = logPanel.getPosition()[1];
            if(newVal){
                newY -= logPanel.getHeight();
            }else{
                newY += logPanel.getHeight() - 60;
            }

            logPanel.animate({
                to: {
                    y: newY
                    //opacity: 0.5
                },
                duration: 500,
                listeners: {
                    afteranimate: function () {
                        //log.debug('finished out animating');
                    }
                }
            });


        });

    }
});
