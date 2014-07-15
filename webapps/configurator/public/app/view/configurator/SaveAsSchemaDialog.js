Ext.define('RtConfigurator.view.configurator.SaveAsSchemaDialog', {
    extend: 'Ext.form.Panel',
    floating: true,
    modal: true,
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
    items: [
        {
            fieldLabel: 'Schema',
            name: 'first',
            allowBlank: false,
            bind: {
                value: '{newSchemaName}'
            }
        },
        {
            fieldLabel: 'Version',
            name: 'last',
            allowBlank: false,
            bind: {
                value: '{newSchemaVersion}'
            }
        }
    ],

    // Reset and Submit buttons
    buttons: [
        {
            text: 'Cancel',
            handler: function () {
                this.up('form').close();
            }
        },
        {
            text: 'Save',
            formBind: true, //only enabled once the form is valid
            disabled: true,

            handler: function () {
                var form = this.up('form').getForm();
                if (form.isValid()) {
                    this.ownerCt.ownerCt.floatParent.controller.onClickSaveAsSchema();
                    this.up('form').close();
                }
            }
        }
    ]
});

