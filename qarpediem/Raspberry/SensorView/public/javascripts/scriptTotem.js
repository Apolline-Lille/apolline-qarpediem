/**
 * Created by carbon on 06/06/2017.
 */
console.log("import scriptTotem.js");



function getMeasurement(){
    return measurement;
}

function getIDSensor(){
    return idSensor;
}

function getTimeFilter(){
    return timeFilter;
}



function addCreateChartButton(){
    if(!(document.getElementById("btnCreateChart"))){
        var body = document.getElementsByTagName("body");
        var btn = document.createElement("button");
        btn.setAttribute("id","btnCreateChart");
        btn.setAttribute("onclick","getDataFromServer()");
        btn.innerHTML = "Create chart";
        body[0].appendChild(btn);
    }
}

function deleteImportButton(){
    $('#importButton').remove();
}

function reinitSelect(){
    var select = document.getElementsByName('IDSensor');
    while(select[0].firstChild){
        select[0].removeChild(select[0].firstChild);
    }
    return select;
}

