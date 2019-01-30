/**
 * Created by carbon on 12/06/2017.
 */
var formQuery = document.getElementById("formButtonQuery");
var measurement, idSensor, timeFilter = 0;
measurement = "chair1";
var getDataLink = "getTableAvailabilityViewData";
var nbChairs = 0;
var values = null;
var tabKeys = null;

function callbackData(data){
    values = data;
    nbChairs = getNbChairsFromData(data);
    tabKeys = getKeysWithoutTime(data);
    createTable(nbChairs);
    resetProgressBar();
    createProgressBar(data);
    createTableConfigurationProgressBar();

}

function getNbChairsFromData(data){
    var j = 0;
    for(var i = 0; i<Object.keys(data).length; i++){
        if(!(Object.keys(data)[i] === "time")){
            j++;
        }
    }
    return j;
}

function getKeysWithoutTime(data) {
    var keys = [];
    for(var i = 0; i<Object.keys(data).length; i++){
        if(!(Object.keys(data)[i] === "time")){
            keys.push(Object.keys(data)[i]);
        }
    }
    keys.sort();
    console.log(keys);
    return keys;
}

function resetProgressBar(){
    $("#progressBar").children().each(function(){
        $(this).remove();
    })
}

function createProgressBar(data){
    var progressBar = document.getElementById("progressBar");
    var rangeBar = document.getElementById("rangeBar");

    var width = getWidthForOnePart(data);

    for(var j = 0; j<data.time.length; j++){
        degre = 0;
        for(var i = 0; i<tabKeys.length; i++){
            if(data[tabKeys[i]].value[j] === "Occupé"){
                degre = degre + 1;
            }
        }
        progressBar.appendChild(createPartOfProgressBar(width, degre, j,tabKeys.length));
    }
    rangeBar.setAttribute("min","0");
    rangeBar.setAttribute("max",""+ (data.time.length-1));
    rangeBar.setAttribute("oninput","hoverTheProgressBar(this.value);");
    rangeBar.setAttribute("onchange","hoverTheProgressBar(this.value);");
}

function createPartOfProgressBar(taille, degre, idTime, nbChaises){
    var v = 100 - (degre / nbChaises * 100);
    var div = document.createElement("div");
    div.setAttribute("class","progress-bar");
    div.setAttribute("role","progressbar");
    div.setAttribute("style","width:"+taille+"%;background-color:hsla(0, 100%, " + v + "%, 1)");
    div.setAttribute("time",idTime);
    return div;
}

function getWidthForOnePart(data){
    return 100/data.time.length;
}

function hoverTheProgressBar(part){
    console.log(part);
    changeTheTable(part);
    changeTheDateProgressionBar(part);
    changeTableConfigurationProgressBar(part);
}

function changeTheTable(part){

    tabStatus = ["-1","0","1"];
    tabColors = ["nothing","someone","chair"];

    for(var i = 0; i<tabKeys.length; i++){
        chair = document.getElementById(tabKeys[i]);
        currClass = chair.getAttribute("class");
        if(currClass === "chairAbove"){
            chair.setAttribute("src","images/"+tabColors[tabStatus.indexOf(values[tabKeys[i]].value[part])]+"Above.png");
            chair.setAttribute("status",""+tabStatus.indexOf(values[tabKeys[i]].value[part]));
        }
        else if(currClass === "rightChair"){
            chair.setAttribute("src","images/"+tabColors[tabStatus.indexOf(values[tabKeys[i]].value[part])]+"Right.png");
            chair.setAttribute("status",""+tabStatus.indexOf(values[tabKeys[i]].value[part]));
        }
        else if(currClass === "leftChair"){
            chair.setAttribute("src","images/"+tabColors[tabStatus.indexOf(values[tabKeys[i]].value[part])]+"Left.png");
            chair.setAttribute("status",""+tabStatus.indexOf(values[tabKeys[i]].value[part]));
        }
        else{
            chair.setAttribute("src","images/"+tabColors[tabStatus.indexOf(values[tabKeys[i]].value[part])]+"Below.png");
            chair.setAttribute("status",""+tabStatus.indexOf(values[tabKeys[i]].value[part]));
        }
    }
}

function changeTheDateProgressionBar(part){
    var dateProgress1 = document.getElementById("dateProgress1");
    var dateProgress2 = document.getElementById("dateProgress2");
    var date = moment(values.time[part]).format("dddd, MMMM Do YYYY, h:mm:ss a");
    var taille = getWidthForOnePart(values) * part;
    if(taille > 50){
        dateProgress1.setAttribute("style","width:"+taille+"%");
        dateProgress2.setAttribute("style","width:"+(100-taille)+"%");
        dateProgress1.innerHTML = date;
        dateProgress2.innerHTML = "";
    }
    else{
        dateProgress1.setAttribute("style","width:"+taille+"%");
        dateProgress2.setAttribute("style","width:"+(100-taille)+"%");
        dateProgress1.innerHTML = "";
        dateProgress2.innerHTML = date;
    }
    console.log(date);
}

function changeTableConfigurationProgressBar(part){
    for(var i = 1 ; i<=nbChairs; i++){
        var chair = document.getElementById("chair"+i);
        var status = chair.getAttribute("status");
        var tableConfigPartBar = document.getElementById("tableConfig"+i);

        if(status === "0") {
            tableConfigPartBar.style.background = "hsla(0, 100%,100%, 1)";
            tableConfigPartBar.style.color = "black";
            console.log(tableConfigPartBar);
        }
        else if(status === "1") {
            tableConfigPartBar.style.background = "hsla(0, 100%,50%, 1)";
            tableConfigPartBar.style.color = "white";
            console.log(tableConfigPartBar);
        }
        else {
            tableConfigPartBar.style.background = "hsla(0, 100%,0%, 1)";
            tableConfigPartBar.style.color = "white";
        }
    }
}

function createTableConfigurationProgressBar(){

    var tableConfigBar = document.getElementById("tableConfigBar");

    for(var i = 0; i<tabKeys.length; i++){
        tableConfigBar.appendChild(createTableConfigurationProgressBarChild(i,nbChairs));
    }
}

function createTableConfigurationProgressBarChild(i, nbChairs) {
    i = i+1;
    var child = document.createElement("div");
    child.setAttribute("class","progress-bar");
    child.setAttribute("role","progressbar");
    child.setAttribute("id","tableConfig"+i);
    child.setAttribute("chair",(i));
    child.setAttribute("onmouseover","showChair(this)");
    child.setAttribute("onmouseout","resetChairHighlighting(this)");
    var width = 100 / nbChairs;
    child.setAttribute("style","width : " + width + "%; background-color:hsla(0, 100%,0%, 1); border : 1px");
    child.innerHTML = "chair " +i;
    return child;
}

function showChair(progress){
    var idChair = progress.getAttribute("chair");
    var chairOnTable = document.getElementById("chair"+idChair);
    chairOnTable.style.backgroundColor = "#000000";
}

function resetChairHighlighting(progress){
    var idChair = progress.getAttribute("chair");
    var chairOnTable = document.getElementById("chair"+idChair);
    chairOnTable.style.backgroundColor = "#ffffff";
}


window.onlad = getImportIDSensorScriptTag();