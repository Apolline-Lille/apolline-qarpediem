/**
 * Created by carbon on 06/06/2017.
 */
console.log("import callbackFunction.js");

function addAllDataSelectionButton(onclick){
    var button = document.createElement("button");
    button.setAttribute("type","button");
    button.setAttribute("class","btn");
    button.setAttribute("onclick",onclick);
    button.innerHTML = "All Measurements";
    return button;
}

function callbackMeasurements(data){
    var measurementButtonRow = getRowDivTag("Measurements","measurementButtonRow");
    console.log(data);
    for(var i=0;i<data.length; i++){
        $(measurementButtonRow).find(".btn-group").each(function(){
            $(this).append(createButtonTag(data[i],"onClickButtonMeasurement(this)"));
        });

    }
    formQuery.appendChild(measurementButtonRow);
    formQuery.appendChild(insertBR("brMeasurementRow"));
}

function callbackIDSensor(data){
    console.log(data);
    var idSensorRow = getRowDivTag("Sensor ID","idSensorButtonRow");
    for(var i=0;i<data.length; i++){
        $(idSensorRow).find(".btn-group").each(function(){
            $(this).append(createButtonTag(data[i],"onClickIDSensor(this)"))
        });
    }
    formQuery.appendChild(idSensorRow);
    formQuery.appendChild(insertBR("brIdSensorRow"));
}

function callbackData(data){
    createChart(data);
}

function insertBR(id){
    var br = document.createElement("br");
    br.setAttribute("id",id);
    return br;
}


