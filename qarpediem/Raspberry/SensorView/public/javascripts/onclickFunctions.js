function importMeasurements(){
    getImportMeasurementsScriptTag();
}

function onClickButtonMeasurement(button){
    measurement = button.innerHTML;
    updateButtons(button);
    deleteButtonsFromBelow("brMeasurementRow");
    getImportIDSensorScriptTag();
}

function onClickIDSensor(button){
    idSensor = button.innerHTML;
    updateButtons(button);
    deleteButtonsFromBelow("brIdSensorRow");
    createTimeFilterButton();
}


function onClickTimeFilterType(button){
    timeFilter = button.innerHTML;
    updateButtons(button);
    deleteButtonsFromBelow("brTimeFilterRow");
    var timeFilterDiv = createDivRowTag("timeFilterButtonRow")
    formQuery.appendChild(timeFilterDiv);
    formQuery.appendChild(insertBR("brTimeFilterButtonRow"));
    getTimeFilterInputsRowDiv(timeFilter,timeFilterDiv);
}


function addShowDataButton(){
    formQuery.appendChild(createButtonTag("Show Data","getDataFromServer()"));
}

function addGetCSVButton(){
    formQuery.appendChild(createDownloadCSVButton("Get CSV","getCSVFromServer()", getParams()));
}

function addSinceTimeFilter(timeFilterDiv){
    addDropdownSinceTimeFilter(timeFilterDiv);
}

function addDropdownSinceTimeFilter(timeFilterDiv) {
    var optionsValue = ["s","m","h","d","w"];
    var optionsLabel = ["seconds","minutes","hours","days","weeks"];
    var divRowTag = createDivRowTag();
    var divDropdown = getDropdownDivTag();
    divDropdown.appendChild(createDropdownButtonTag());
    var ulDropdown = getUlDropdownMenuTag();
    for(var i = 0; i<optionsValue.length; i++){
        ulDropdown.appendChild(createDropdownMenuItemsTag(optionsLabel[i],optionsValue[i]));
    }
    divDropdown.appendChild(ulDropdown);
    timeFilterDiv.appendChild(getLabelForButtonRowDiv('"Since" timer selection'));
    timeFilterDiv.appendChild(getInputTimeFilterTag());
    timeFilterDiv.appendChild(divDropdown);
    formQuery.appendChild(divRowTag);
}

function setUnitSelectionTo(label, value) {
    var button = document.getElementById("buttonTimeFilterSelectionUnit");
    button.setAttribute('value',value);
    button.innerHTML = label;
}
function addBetweenTimeFilter(timeFilterDiv){
    timeFilterDiv.appendChild(getLabelForButtonRowDiv('"Between" time filter'));
    timeFilterDiv.appendChild(createDateTimeInputTag("inputTimeBetween1"));
    timeFilterDiv.appendChild(createDateTimeInputTag("inputTimeBetween2"));
}

function getDataFromServer(){
    var params = getParams();
    console.log(params);
    getImportDataScriptTag(params);
}

function getCSVFromServer(){
    var params = getParams();

}

function getParams(){

    var measurement = getMeasurement();
    var idSensor = getIDSensor();
    var timeFilter = getTimeFilter();

    var returnString = "measurement="+measurement+"&idsensor="+idSensor;

    if(timeFilter !== "None"){
        if(timeFilter === "Since") {
            var nTime = document.getElementById("inputTimeFilter").value;
            var timeUnit = document.getElementById("buttonTimeFilterSelectionUnit").value;
            return returnString+"&timeFilter="+timeFilter+"&nTime="+nTime+"&timeUnit="+timeUnit;
        }
        else{
            var time1 = document.getElementById("inputTimeBetween1").value;
            var time2 = document.getElementById("inputTimeBetween2").value;

            date1 = new Date(datetimelocalToDateObject(time1));
            date2 = new Date(datetimelocalToDateObject(time2));

            if(date1.getTime() > date2.getTime()){
                time1 = dateToDateTimeLocal(date2);
                time2 = dateToDateTimeLocal(date1);
            }

            return returnString+"&timeFilter="+timeFilter+"&time1="+time1+"&time2="+time2;

        }
    }
    else {
        return returnString+"&timeFilter=None";
    }
}/**
 * Created by carbon on 26/06/2017.
 */
