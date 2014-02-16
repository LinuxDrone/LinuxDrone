var commonModuleParamsDefinition = [
    {
        name: "Task Priority",
        "displayName": {
            "en": "Task Priority",
            "ru": "Task Priority"
        },
        description: {ru: "Приоритет потока (задачи) xenomai"},
        defaultValue: 80,
        unitMeasured: "%",
        value: 80
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
        defaultValue: 20,
        unitMeasured: "Ms",
        value: 20
    },
    {
        name: "Notify on change",
        "displayName": {
            "en": "Notify on change",
            "ru": "Notify on change"
        },
        type: "bool",
        unitMeasured: "",
        value: true
    }
]
