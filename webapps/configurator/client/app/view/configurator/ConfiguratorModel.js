/**
 * Created by vrubel on 14.07.14.
 */
Ext.define('RtConfigurator.view.configurator.ConfiguratorModel', {
    extend: 'Ext.app.ViewModel',

    alias: 'viewmodel.configurator',

    data: {
        newSchemaName: '',
        newSchemaVersion: '',
        nameOfSelectedInstance: 'Properties',
        isJustSelectedInstance: false, // Этотакой костыль. Переменная устанавливается в true в момент выбора инстанса на схеме.
        // После изменеия переменной nameOfSelectedInstance, которой присваивается имя инстанса, флаг сбрасывается.
        // Это сделано для того, чтобы отличать факт изменения перменной nameOfSelectedInstance от действия выбора инстанса
        // от действия смены именя инстанса в панели свойств

        currentModuleProps: {},

        // Тип выбранного на схеме линка
        typeSelectedLink:'',

        // Скрывать панель свойств связи
        hideLinkProperties:true,
        // Скрывать панели свойств выбранного инстанса
        hideInstanceProperties:true
    },

    formulas: {
        disableDeleteModule: function (get) {
            return get('nameOfSelectedInstance') == 'Properties';
        }
    }

});