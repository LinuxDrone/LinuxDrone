/**
 * Created by vrubel on 30.07.14.
 */
Ext.define('RtConfigurator.view.configurator.dialogs.ImportSchemaDialog', {
    extend: 'Ext.form.Panel',

    requires: [
        'Ext.form.field.File'
    ],

    reference: 'importdialog',

    floating: true,
    modal: true,
    title: 'Import Schema',
    bodyPadding: 5,
    width: 350,

    // Fields will be arranged vertically, stretched to full width
    layout: 'anchor',
    defaults: {
        anchor: '100%'
    },

    // The fields
    items: [
        {
            xtype: 'filefield',
            name: 'file',
            labelWidth: 50,
            msgTarget: 'side',
            allowBlank: false,
            anchor: '100%',
            buttonText: 'Select File...',
            bind: {
                //value: '{newSchemaVersion}'
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
            text: 'Import',
            formBind: true, //only enabled once the form is valid
            disabled: true,
            handler: 'onImportSchema'
        }
    ]
});

