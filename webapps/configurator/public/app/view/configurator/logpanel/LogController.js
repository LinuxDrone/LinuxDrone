/**
 * Created by vrubel on 29.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogController', {
    extend: 'Ext.app.ViewController',

    alias: 'controller.logpanel',

    init: function () {
        var model = this.getView().getViewModel();

        model.bind('{expanded}', function(newVal){

            var logPanel = this.getView();
            logPanel.animate({
                to: {
                    y: logPanel.getPosition()[1] - logPanel.getHeight() + 30
                    //opacity: 0.5
                },
                duration: 500,
                listeners: {
                    afteranimate: function () {
                        log.debug('finished out animating');
                    }
                }
            });


        });

    }
});
