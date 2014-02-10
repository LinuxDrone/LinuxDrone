{
    "type": "module_def",
    "name": "Bmp180",  
    "version": 3,
    "task_priority": 80,
    "task_period": 200,
    "paramsSchema":{
        "type":"object",
        "id": "http://jsonschema.net",
        "properties":{
            "bus_type": {
                "type":"number",
                "required":true
            },
            "bus_name": {
                "type":"string",
                "required":true
            }
        }
    },
    "outputs":[
        {
            "name": "Pressure+Temp",
            "Schema":{
                "type":"object",
                "id": "http://jsonschema.net",
                "properties":{
                    "pressure": {
                        "type":"number",
                        "required":true
                    },
                    "temperature": {
                        "type":"number",
                        "required":true
                    }
                }
            }
        }
    ]
}
