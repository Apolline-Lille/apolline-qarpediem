/**
 * Created by carbon on 08/06/2017.
 */

function getImportMeasurementsScriptTag(){
    var script = document.createElement("script");
    script.setAttribute('name','importMeasurements');
    script.setAttribute("src","/getMeasurements?callbackFunction=callbackMeasurements");
    var body = document.getElementsByTagName("body");
    body[0].appendChild(script);
}

function getImportIDSensorScriptTag(){
    var script = document.createElement("script");
    script.setAttribute('name','importIDSensor');
    script.setAttribute("src","/getIDSensor?callbackFunction=callbackIDSensor&measurement="+measurement);
    var body = document.getElementsByTagName("body");
    body[0].appendChild(script);
}

function createButtonTag(data, onclick){
    var button = document.createElement("button");
    button.setAttribute("type","button");
    button.setAttribute("class","btn");
    button.setAttribute("onclick",onclick);
    button.innerHTML = data;
    return button;
}

function createDownloadCSVButton(data, onclick, params){
    var button = document.createElement("button");
    button.setAttribute("type","button");
    button.setAttribute("class","btn");
    var a = document.createElement("a");
    a.setAttribute("href","/downloadCSV?"+params);
    a.setAttribute("download",getMeasurement() + ".csv");
    a.innerHTML = data;
    button.appendChild(a);
    return button;
}

function getButtonRowDivTag(id){
    var buttonRow = document.createElement("div");
    buttonRow.setAttribute("id", id);
    buttonRow.setAttribute("class","col-md-10 btn-group");
    return buttonRow;
}

function getRowDivTag(label,id) {
    var divrow = createDivRowTag();
    divrow.appendChild(getLabelForButtonRowDiv(label));
    divrow.appendChild(getButtonRowDivTag(id));
    console.log(divrow);
    return divrow;
}

function createDivRowTag(){
    var div = document.createElement("div");
    div.setAttribute("class","row");
    return div;
}

function getLabelForButtonRowDiv(value){
    var label = document.createElement("label");
    label.setAttribute("class","col-md-2");
    label.innerHTML = value;
    return label;
}


function createTimeFilterButton(){
    var options = ["None","Since","Between"];
    var timeFilterRow = getRowDivTag("Time filter","timeFilterSelectionButtonRow");
    for(var i = 0; i<options.length; i++){
        $(timeFilterRow).find(".btn-group").each(function(){
            $(this).append(createButtonTag(options[i],"onClickTimeFilterType(this)"))
        });
    }
    formQuery.appendChild(timeFilterRow);
    formQuery.appendChild(insertBR("brTimeFilterRow"));
}

function getTimeFilterInputsRowDiv(value,timeFilterDiv) {
    switch(value){
        case "Since" :
            addSinceTimeFilter(timeFilterDiv);
            addShowDataButton();
            addGetCSVButton();
            break;
        case "Between" :
            addBetweenTimeFilter(timeFilterDiv);
            addShowDataButton();
            addGetCSVButton();
            break;
        case "None" :
            addShowDataButton();
            addGetCSVButton();
            break;
    }
}

function getDropdownDivTag(){
    var divDropdown = document.createElement("div");
    divDropdown.setAttribute("class","col-md-8 dropdown");
    return divDropdown;
}

function getUlDropdownMenuTag(){
    var ulDropdown = document.createElement("ul");
    ulDropdown.setAttribute("class","dropdown-menu");
    return ulDropdown;
}

function createDropdownMenuItemsTag(label, value){
    var option = document.createElement("li");
    var a = document.createElement("a");
    a.setAttribute("onclick","setUnitSelectionTo('"+label+"','"+value+"')");
    a.innerHTML = label;
    option.appendChild(a);
    return option;
}

function getInputTimeFilterTag() {
    var div = document.createElement("div");
    div.setAttribute("class","col-md-2");
    var inputTime = document.createElement("input");
    inputTime.setAttribute("class","form-control");
    inputTime.setAttribute("id","inputTimeFilter");
    div.appendChild(inputTime);
    return div;
}

function createDropdownButtonTag() {
    var buttonDropdown = document.createElement("button");
    buttonDropdown.setAttribute("id","buttonTimeFilterSelectionUnit");
    buttonDropdown.setAttribute("class","btn btn-primary dropdown-toggle");
    buttonDropdown.setAttribute("type","button");
    buttonDropdown.setAttribute("data-toggle","dropdown");
    buttonDropdown.setAttribute("value","none");
    buttonDropdown.innerHTML = "Unit selection";
    return buttonDropdown;
}

function createDateTimeInputTag(id){
    var input = document.createElement("input");
    input.setAttribute("type","datetime-local");
    input.setAttribute("class","form_datetime");
    input.setAttribute("id",id);
    return input;
}

function getImportDataScriptTag(params){
    var body = document.getElementsByTagName("body");
    var script = document.createElement("script");
    script.setAttribute('name','importData');
    script.setAttribute("src","/"+getDataLink+"?callbackFunction=callbackData&"+params);
    body[0].appendChild(script);
}

function updateButtons(button){
    $(button).parent().find("button").each(function(){
        $(this).attr("class","btn");
    });
    $(button).attr("class","btn btn-success");
}

function deleteButtonsFromBelow(currentID){
    var declencheur = 0;
    $("#formButtonQuery").children().each(function(){
        var id = $(this).attr('id');
        if(id===currentID){
            declencheur++;
        }
        if((id!==currentID)&&(declencheur>0)){
            $(this).remove();
        }
    })
}