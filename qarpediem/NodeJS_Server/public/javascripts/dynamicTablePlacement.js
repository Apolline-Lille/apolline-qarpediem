/**
 * Created by carbon on 30/06/2017.
 */

var container = document.getElementById("connectedTable");
var idChairs = 0;

function createTable(nbChair){

    deletePreviousTable();

    idChairs = nbChair;
    var nbChairAbove = determineNumberOfChairAbove(nbChair);
    var nbChairBelow = determineNumberOfChairBelow(nbChair);
    var nbChairAround = determineNumberOfChairAround(nbChair);

    console.log("nbChair above : " + nbChairAbove);
    console.log("nbChair around : " + nbChairAround);
    console.log("nbChair below : " + nbChairBelow);


    createChairLine("chairAbove",nbChairAbove);
    createAroundChairAndTable(nbChairAround);
    createChairLine("chairBelow",nbChairBelow);
}

function deletePreviousTable(){
    $(container).empty();
}

function showLegend(){
    var legend = document.getElementById("legend");
    legend.setAttribute("style","display : ");
}

function determineNumberOfChairAbove(nbChair){
    return Math.round(nbChair / 3);
}

function determineNumberOfChairAround(nbChair){
    return Math.round(nbChair / 6);
}

function determineNumberOfChairBelow(nbChair){
    var above = determineNumberOfChairAbove(nbChair);
    var around = determineNumberOfChairAround(nbChair);
    var chairBelow = (above + 2 * around);
    chairBelow = nbChair - chairBelow;
    return chairBelow;
}

function createChairLine(position,nbChair){
    var divRow = document.createElement('div');
    divRow.setAttribute("class","row");
    divRow.setAttribute("style","padding: 0px; margin: 0; text-align: center");
    container.appendChild(divRow);
    createBootstrapChairLine(divRow,position,nbChair);
}

function createBootstrapChairLine(divRow,position,nbChair){
    var chairBlockDiv = document.createElement("div");
    chairBlockDiv.setAttribute("class","col-sm-offset-4 col-sm-4 chaise");
    chairBlockDiv.setAttribute("style","position: relative; padding:0;");
    divRow.appendChild(chairBlockDiv);
    for(var i=0; i<nbChair; i++){
        insertChair(chairBlockDiv,position,nbChair);
    }

}

function insertChair(chairBlockDiv,position,nbChair){
    var img = document.createElement("img");
    var posPic = getPicofChairDependingPosition();
    img.setAttribute("src","images/nothing"+posPic+".png");
    img.setAttribute("style","max-width : " + 90/nbChair + "%;padding:1px");
    img.setAttribute("id","chair"+idChairs);
    idChairs--;
    img.setAttribute("class",position);
    chairBlockDiv.appendChild(img);
}

function getPicofChairDependingPosition(position){
    var id;
    if(position === "chairAbove") id = "Above";
    else if (position === "chairBelow") id = "Below";
    else if (position === "leftChair") id = "Left";
    else id = "Right";
    return id;
}

function createAroundChairAndTable(nbChair) {
    var divRow = document.createElement("div");
    divRow.setAttribute("class","row");
    divRow.setAttribute("style","padding: 0px; margin: 0px");
    container.appendChild(divRow);
    createAroundChair(divRow,"leftChair",nbChair);
    createTableBlock(divRow);
    createAroundChair(divRow,"rightChair",nbChair);
}

function createAroundChair(divRow,position,nbChair){
    var chairBlockDiv = document.createElement("div");
    chairBlockDiv.setAttribute("class","col-sm-4 aroundChair");
    divRow.appendChild(chairBlockDiv);
    for(var i=0; i< nbChair; i++){
        insertChairAround(chairBlockDiv,position,nbChair);
        insertClearFixDiv(chairBlockDiv);
    }
}

function insertChairAround(chairBlockDiv,position,nbChair){
    var posPic = getPicofChairDependingPosition(position);
    var img = document.createElement("img");
    img.setAttribute("src","images/nothing"+posPic+".png");
    img.setAttribute("class",position);
    img.setAttribute("id","chair"+idChairs);
    idChairs--;
    var height = Math.floor(194 / nbChair);
    if(nbChair > 2){
        img.setAttribute("style","max-height :" + height + "px;padding : 1px;");
        console.log("Insertion image");
    }
    else{
        var margin = (194/nbChair-72)/2;
        img.setAttribute("style","max-height :" + height + "px; margin-top :" + margin + "px; margin-bottom :" + margin +"px; padding : 1px;");
    }
    chairBlockDiv.appendChild(img);
}

function insertClearFixDiv(chairBlockDiv){
    var div = document.createElement("div");
    div.setAttribute("class","clearfix");
    chairBlockDiv.appendChild(div);
}

function createTableBlock(divRow){
    var div = document.createElement("div");
    div.setAttribute("class","col-sm-4");
    div.setAttribute("style","padding: 0");
    var border = document.createElement("div");
    border.setAttribute("style","border : solid 1px black; border-bottom: none; border-top: none;");
    var img = document.createElement("img");
    img.setAttribute("src","images/tablemilieu.png");
    img.setAttribute("class","tableConnectee");
    border.appendChild(img);
    div.appendChild(border);
    divRow.appendChild(div);
}