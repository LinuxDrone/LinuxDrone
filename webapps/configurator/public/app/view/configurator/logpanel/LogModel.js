/**
 * Created by vrubel on 29.07.14.
 */
Ext.define('RtConfigurator.view.configurator.logpanel.LogModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.logpanel',

    data: {
        // true когда панелька выдвинута
        expanded: false,

        telemetryLabelBackground: 'red',
        logLabelBackground: 'red'
    }
});