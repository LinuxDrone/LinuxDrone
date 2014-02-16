var commonModuleParamsDefinition = [
    {
        name: "Task Priority",
        "displayName": {
            "en": "Task Priority",
            "ru": "Task Priority"
        },
        type: "number",
        required: true,
        description: {ru: "Приоритет потока (задачи) xenomai"},
        unitMeasured: "%",
        defaultValue: 80
    },
    {
        name: "Task Period",
        "displayName": {
            "en": "Task Period",
            "ru": "Task Period"
        },
        type: "number",
        required: true,
        description: {ru: "время между двумя вызовами бизнес функции (микросекунд) 0  - не зависать на очереди в ожидании данных -1 - зависать навечно, до факта появления данных в очереди"},
        unitMeasured: "Ms",
        defaultValue: 20
    },
    {
        name: "Notify on change",
        "displayName": {
            "en": "Notify on change",
            "ru": "Notify on change"
        },
        type: "boolean",
        unitMeasured: "",
        defaultValue: true
    }
];

if(this.module && this.module.exports)
{
    module.exports.commonModuleParamsDefinition = commonModuleParamsDefinition;
}
