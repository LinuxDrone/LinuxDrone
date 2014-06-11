(function(exports){

    exports.commonModuleParamsDefinition = [
        {
            name: "Task Priority",
            "displayName": {
                "en": "Task Priority",
                "ru": "Task Priority"
            },
            type: "char",
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
            type: "long long",
            required: true,
            description: {ru: "время между двумя вызовами бизнес функции (микросекунд) 0  - не зависать на очереди в ожидании данных -1 - зависать навечно, до факта появления данных в очереди"},
            unitMeasured: "Ms",
            defaultValue: 20
        },
        {
            name: "Transfer task period",
            "displayName": {
                "en": "Transfer task period",
                "ru": "Transfer task period"
            },
            type: "long long",
            required: true,
            description: {ru: "время между двумя вызовами бизнес функции (микросекунд) 0  - не зависать на очереди в ожидании данных -1 - зависать навечно, до факта появления данных в очереди"},
            unitMeasured: "Ms",
            defaultValue: 20
        }
    ];


})(typeof exports === 'undefined'? this['ModulesCommonParams']={}: exports);
