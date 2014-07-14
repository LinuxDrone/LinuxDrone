
Ext.define('RtConfigurator.view.svg.SaveAsSchemaDialog', {
    extend: 'Ext.form.Panel',
    floating: true,
    modal : true,
    title: 'Save Schema As',
    bodyPadding: 5,
    width: 350,

    // Fields will be arranged vertically, stretched to full width
    layout: 'anchor',
    defaults: {
        anchor: '100%'
    },

    // The fields
    defaultType: 'textfield',
    items: [{
        fieldLabel: 'Schema',
        name: 'first',
        allowBlank: false,
        bind:{
            value: '{newSchemaName}'
        }
    },{
        fieldLabel: 'Version',
        name: 'last',
        allowBlank: false
    }],

    // Reset and Submit buttons
    buttons: [{
        text: 'Cancel',
        handler: function() {
            this.up('form').close();
            //this.up('form').getForm().reset();
        }
    }, {
        text: 'Save',
        formBind: true, //only enabled once the form is valid
        disabled: true,

        handler: 'onClickSaveAsSchema'

        /*
        handler: function() {
            var form = this.up('form').getForm();
            if (form.isValid()) {
                console.log(form);
            }
        }*/
    }]
});

