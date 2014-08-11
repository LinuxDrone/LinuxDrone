/**
 * Created by vrubel on 11.07.14.
 */

Ext.define('RtConfigurator.data.MongoIdGenerator', {
    extend: 'Ext.data.identifier.Generator',
    alias: 'data.identifier.mongoidgenerator',

    generate: function () {
        return bson().ObjectID.createPk().toJSON();
    }
});