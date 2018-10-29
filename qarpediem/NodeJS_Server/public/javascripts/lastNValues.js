/**
 * Created by lepallex on 21/09/2016.
 */



urlN = "http://www.lifl.fr/miny/totem/server/lastNValues.php";
dataTypeN = "jsonp";
methodN = "get";

function recupererDernieresNValeurs (idCapteur, nombreValeurs, callback) {
    console.log("RecupNValeurs");
    var body = document.getElementsByTagName("body");
    var script = document.createElement("script");
    script.setAttribute('name','importLast'+nombreValeurs+'ValuesFor'+idCapteur);
    script.setAttribute("src","/lastNValues?callbackFunction="+callback+"&sensorId="+idCapteur+"&n="+nombreValeurs);
    body[0].appendChild(script);
    /*
    var allData = {
        url: urlN,
        dataType: dataTypeN,
        method: methodN,
        data: { sensorId : idCapteur , N : nombreValeurs },
        success: succes,
        error: error
    };
    $.ajax(allData);*/
}

function error(event) {
    console.log("Erreur : la requête AJAX n'a pas abouti : ");
    console.log(event);
}

google.charts.load('current', {'packages':['corechart']});


var legends = {
    "0735" : "Capteurs météo en bas du Totem",
    "0736" : "Capteurs météo en haut du Totem",
    "1878" : "Capteurs de déformation en haut du Totem",
    "1876" : "Capteurs de déformation presque en bas du Totem",
    "1877" : "Capteurs de déformation en bas du Totem"
}

function afficherLesGraphiquesPour (idCapteur) {
    switcher.retirerMesures();
    switcher.afficherGraphiques();
    switcher.textePourLegend (legends[idCapteur]);


    recupererDernieresNValeurs(idCapteur, 60, "dessinerGraphiques");


    switcher.body.onclick = function () {
        console.log("go");
        switcher.retirerGraphiques();
        switcher.afficherMesures();
        switcher.body.onclick = null;
        switcher.textePourLegend ("Totem IRCICA");
    }
}

var titresGraphiques = {
    "humidity" : "Humidité",
    "temperature" : "Température",
    "sensorRaw" : "Données brutes",
    "vBat" : "Batterie",
    "delta" : "Déformation",
    "dewPoint" : "Point de Rosée",
    "boardTemp" : "Température du boitier"
}

function dateTronquee (date) {
    return "("+date.substring(8).replace(" ",") ");
}
function dessinerGraphiques(valeurs) {
    var tableau1 = new Array();
    var tableau2 = new Array();
    var tableau3 = new Array();
    var tableau4 = new Array();
    console.log("valeurs");
    console.log(valeurs);

    for (var i=0;i<valeurs.length;i++) {
        tableau1.splice(1, 0,[dateTronquee(valeurs[i][0]),valeurs[i][1]]);
        tableau2.splice(1, 0,[dateTronquee(valeurs[i][0]),valeurs[i][2]]);
        tableau3.splice(1, 0,[dateTronquee(valeurs[i][0]),valeurs[i][3]]);
        tableau4.splice(1, 0,[dateTronquee(valeurs[i][0]),valeurs[i][4]]);
    }

    console.log(tableau1);

    //dessinerUnGraphe(valeurs, 'Mesures de tous les Capteurs du Totem', 'grapheTousLesCapteurs');

    dessinerUnGraphe(tableau1, titresGraphiques[tableau1[0][1]]+ " en "+unitesChamps[tableau1[0][1]], 'grapheCapteur1');
    dessinerUnGraphe(tableau2, titresGraphiques[tableau2[0][1]]+ " en "+unitesChamps[tableau2[0][1]], 'grapheCapteur2');
    dessinerUnGraphe(tableau3, titresGraphiques[tableau3[0][1]]+ " en "+unitesChamps[tableau3[0][1]], 'grapheCapteur3');
    dessinerUnGraphe(tableau4, titresGraphiques[tableau4[0][1]]+ " en "+unitesChamps[tableau4[0][1]], 'grapheCapteur4');
}

function dessinerUnGraphe (tableau, titre, idHTML) {
    console.log(tableau);


    var data = google.visualization.arrayToDataTable(tableau);

    var options = {
        title: titre,
        curveType: 'function',
        legend: { position: 'bottom' }//,
        //backgroundColor: '#E4E4E4'
    };

    var chart = new google.visualization.LineChart(document.getElementById(idHTML));

    chart.draw(data, options);
}
