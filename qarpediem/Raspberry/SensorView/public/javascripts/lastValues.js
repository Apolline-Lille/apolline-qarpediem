/**
 * Created by lepallex on 21/09/2016.
 */

// recupererDernieresValeurs("0735",function (valeurs) {afficherValeurs(valeurs,"0735");});

function ajouterBalise(parent,baliseType,classe) {
    var balise=document.createElement(baliseType);
    if (classe)
        balise.className=classe;
    parent.appendChild(balise);
    return balise;
}

function ajouterTexte (parent, texte) {
    var baliseTexte=document.createTextNode(texte);
    parent.appendChild(baliseTexte);
    return baliseTexte;
}

url = window.location.protocol + "//" + window.location.host + "/lastValues"
dataType = "json";
method = "get";

function recupererDernieresValeurs (idCapteur, callback) {
    var body = document.getElementsByTagName("body");
    var script = document.createElement("script");
    script.setAttribute('name','importLastValuesFor'+idCapteur);
    script.setAttribute("src","/lastValues?callbackFunction="+callback+"&sensorId="+idCapteur);
    body[0].appendChild(script);

    /*var allData = {
        url: url,
        dataType: dataType,
        method: method,
        data: { sensorId : idCapteur },
        success: succes,
        error: error
    };
    $.ajax(allData);*/
}

function error(event) {
    console.log("Erreur : la requête AJAX n'a pas abouti : ");
    console.log(event);
}

var labelsChamps = {
    "humidity" : "Humidité",
    "temperature" : "Température",
    "sensorRaw" : "Données brutes",
    "vBat" : "Batterie",
    "delta" : "Déformation",
    "dewPoint" : "Point de Rosée",
    "boardTemp" : "T° du boitier"
};

var unitesChamps = {
    "humidity" : "%",
    "temperature" : "°C",
    "sensorRaw" : String.fromCharCode(181)+"m/m",
    "vBat" : "V",
    "delta" : String.fromCharCode(181)+"m/m",
    "dewPoint" : "°C",
    "boardTemp" : "°C"
};


function afficherValeurs (valeurs, idCapteur) {
    var panelBody = document.getElementById("Valeurs"+idCapteur);
    var panelFoot = document.getElementById("Heure"+idCapteur);
    for (var cle in valeurs) {
        if (cle!="date") {
            ajouterTexte(panelBody, labelsChamps[cle] + " : " + valeurs[cle]+" "+unitesChamps[cle]);
            ajouterBalise(panelBody, "BR");
        }
        else {
            console.log(valeurs);
            ajouterTexte(panelFoot, "Le " + valeurs[cle].split(" ")[0].substring(8)+"/"+valeurs[cle].split(" ")[0].substring(5,7));
            //ajouterBalise(panelFoot, "BR");
            ajouterTexte(panelFoot, " à " + valeurs[cle].split(" ")[1]);
        }
    }
}