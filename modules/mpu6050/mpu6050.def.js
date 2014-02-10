{
    "type": "module_def",
    "name": "GY-87",
    "version": 6,
    "task_priority": 80,
    "task_period": 200,
    "notifyOnChange": false,
    "description": {
        "en": "GY-87 sensors set",
        "ru": "Набор датчиков на плате GY-87"
    },
    "paramsSchema": {
        "type": "object",
        "id": "http://jsonschema.net",
        "properties": {
            "i2cDevName": {
                "type": "string",
                "required": true,
                "defaultValue": "/dev/i2c-1",
                "validValues" : ["/dev/i2c-1","/dev/i2c-2"],
                "displayName": {
                    "en": "Bus Name",
                    "ru": "Имя шины"
                },
                "description": {
                    "en": "Bus Name",
                    "ru": "Имя устройства шины"
                }
            }
        }
    },
    "outputs": [
        {
            "name": "Gyro+Accel+Mag+Temp",
            "Schema": {
                "type": "object",
                "id": "http://jsonschema.net",
                "properties": {
                    "accelX": {
                        "type": "number",
                        "required": true
                    },
                    "accelY": {
                        "type": "number",
                        "required": true
                    },
                    "accelZ": {
                        "type": "number",
                        "required": true
                    },
                    "gyroX": {
                        "type": "number",
                        "required": true
                    },
                    "gyroY": {
                        "type": "number",
                        "required": true
                    },
                    "gyroZ": {
                        "type": "number",
                        "required": true
                    },
                    "magX": {
                        "type": "number",
                        "required": true
                    },
                    "magY": {
                        "type": "number",
                        "required": true
                    },
                    "magZ": {
                        "type": "number",
                        "required": true
                    },
                    "temperature": {
                        "type": "number",
                        "required": true
                    }
                }
            }
        },
        {
            "name": "Baro",
            "Schema": {
                "type": "object",
                "id": "http://jsonschema.net",
                "properties": {
                    "pressure": {
                        "type": "number",
                        "required": true
                    }
                }
            }
        }
    ]
}
