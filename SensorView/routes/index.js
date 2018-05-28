var express = require('express');
var router = express.Router();
var moment = require('moment');
var json2csv = require('json2csv');
require('array.prototype.move');	// CommonJS syntax
var Busboy = require('busboy');

//INSTANCIATION DE L'OBJET INFLUXDB
var influx = require('influx');
var influxDB = null;

//----------------------------------------------------------------------------------------------------------------------
//-------------------------------- ACCÈS AUX PAGES WEB ET CONNEXION À LA BASE DE DONNÉES -------------------------------
//----------------------------------------------------------------------------------------------------------------------

router.get('/',function(req,res){
    res.render('accueil');
});

router.get('/totemView', function(req, res) {
    connectToInfluxDB('totem');
    res.render('totemView');
});

router.get('/connectedTableView',function(req,res){
    connectToInfluxDB('connectedtables');
    res.render('connectedTableView')
});

router.get('/tableAvailabilityView', function (req,res){
    connectToInfluxDB('connectedtables');
    res.render('tableAvailabilityView');
});

router.get('/tableAvailabilityViewV12', function (req,res){
    connectToInfluxDB('connectedtables');
    res.render('tableAvailabilityV12');
});

router.get('/airQualityView', function (req,res){
    connectToInfluxDB('airquality');
    res.render('airQualityView');
});

router.get('/sonometerView', function (req,res){
    connectToInfluxDB('airquality');
    res.render('sonometerView');
});

function connectToInfluxDB(database){
    influxDB = new influx.InfluxDB({
        host : 'lilliad.apisense.io',
        port : 8086,
        database : database,
        username : 'lilliad',
        password : 'lilliad'
    });
}

//----------------------------------------------------------------------------------------------------------------------
//-------------------------------- REQUÊTES POUR RÉCUPÉRATION DES DONNÉES SUR LA BDD -----------------------------------
//----------------------------------------------------------------------------------------------------------------------

router.get('/getMeasurements', function(req,res){
    var callbackMeasurements = req.query.callbackFunction;
    influxDB.query('SHOW MEASUREMENTS').then(
        function (results) {
            var measurements = [];
            for(var i=0; i<results.length; i++){measurements.push(results[i].name);}
            res.send(""+ callbackMeasurements + '(' + JSON.stringify(measurements) + ');');});});

router.get('/getIDSensor', function(req,res){
    var callback = req.query.callbackFunction;
    var critere = req.query.measurement;
    influxDB.query('SHOW TAG VALUES FROM ' + critere + ' WITH KEY="device"').then(
        function (results) {
            var idNodes = [];
            for(var i=0; i<results.length; i++){ idNodes[i] = results[i].device; }
            res.send(""+ callback + '(' + JSON.stringify(idNodes) + ');');});});

router.get('/getDataFromServer', function(req,res){
    var callback = req.query.callbackFunction;
    var query = getQueryFromParams(req.query);
    influxDB.query(query).then(
        function (results) {
            var data = {
                time : [],
                values : []
            };
            for(var i=0; i<results.length; i++){
                data.time[i] = moment(results[i].time);
                data.values[i] = results[i].value;
            }
            data.measurement = req.query.measurement;
            data.device = req.query.device;
            res.status(200).send(""+ callback + '(' + JSON.stringify(data) + ');');});});

router.get('/getTableAvailabilityViewData', function(req,res){
    var callback = req.query.callbackFunction;
    influxDB.query("SHOW MEASUREMENTS").then(
        function(results){
            console.log(results);
            var measurement = "";
            for(var i = 0; i<results.length; i++){
                if(results[i].name.includes("chair")){
                    if((measurement === "")){
                        measurement = measurement + results[i].name ;
                    }
                    else{
                        measurement = measurement + ", " + results[i].name ;
                    }
                }
            }
            req.query.measurement = measurement;
            console.log(req.query.measurement);
            var query = getQueryFromParams(req.query);
            influxDB.query(query).then(
                function (results) {
                    var etatChaise;
                    var data = {
                        time : []
                    };
                    for(var i=0; i<results.groupRows.length; i++){
                        data[results.groupRows[i].name] = {value : []};
                        for(var k=0; k<results.groupRows[i].rows.length; k++){
                            if(!(inArray(data.time, moment(results.groupRows[i].rows[k].time).format()))){
                                data.time.push(moment(results.groupRows[i].rows[k].time).format());
                            }
                        }
                    }
                    data.time.sort();
                    var nbChaise = getNbChaise(data);
                    for(var l=0; l<data.time.length; l++){
                        for(var m=0; m<nbChaise.length; m++){
                            etatChaise = getEtatAt(results.groupRows[m].rows,data.time[l]);
                            if(etatChaise === false){
                                data[nbChaise[m]].value[l] = data[nbChaise[m]].value[l-1];
                            }
                            else{
                                data[nbChaise[m]].value[l] = etatChaise;
                            }
                        }
                    }
                    res.status(200).send(callback + '(' + JSON.stringify(data) + ');');})});});

router.get('/downloadCSV', function (req,res){
    var query = getQueryFromParams(req.query);
    console.log(query);
    influxDB.query(query).then(
        function (results) {
            var csv = {
                fields : ["device","time", "value"],
                values : []
            };
            for(var i=0; i<results.length; i++) {
                csv.values.push({
                    "device": req.query.device,
                    "time": moment(results[i].time).utc(),
                    "value": results[i].value})}
            res.status(200).type("text/csv").send(generateCSVString(csv));});});

function getQueryFromParams(params) {
    var measurement = params.measurement;
    var idNode = params.device;
    var timeFilter = params.timeFilter;
    var query = "";
    if (timeFilter === "None") query = "SELECT * FROM " + measurement + " WHERE device='" + idNode + "'";
    else if (timeFilter === "Since") {
        var ntime = params.nTime;
        var timeUnit = params.timeUnit;
        query = "SELECT * FROM " + measurement + " WHERE device='" + idNode + "' AND time> now() - " + ntime + timeUnit;
    }
    else if (timeFilter === "Between") {
        var time1 = moment(params.time1).utc().format();
        var time2 = moment(params.time2).utc().format();
        query = "SELECT * FROM " + measurement + " WHERE device='" + idNode + "' AND time>'" + time1 + "' AND time<'" + time2 + "'";
    }
    return query;}

function generateCSVString(csv){ return json2csv({data : csv.values, fields : csv.fields}); }



function inArray(array, el) {
    for ( var i = array.length; i--; ) { if ( array[i] === el ) return true; }
    return false;
}


function getEtatAt(array, el){
    m = Math.floor(array.length / 2);
    if(array.length === 1){
        if(moment(array[0].time).isSame(moment(el))) return array[0].value;
        else return false
    }
    else{
        if (moment(el).isAfter(moment(array[m].time))) return getEtatAt(array.slice(m, array.length), el);
        else if (moment(el).isBefore(moment(array[m].time))) return getEtatAt(array.slice(0,m), el);
        else if(moment(array[m].time).isSame(moment(el))) return array[m].value;
    }
}

function getNbChaise(data){
    var nbChaise = [];
    for(var i = 0; i<Object.keys(data).length; i++){
        if(!(Object.keys(data)[i] === "time"))nbChaise.push(Object.keys(data)[i]);}
    return nbChaise;
}

//----------------------------------------------------------------------------------------------------------------------
//-------------------------------- REQUÊTES POUR ENVOI DES DONNÉES À LA BASE DE DONNÉES --------------------------------
//----------------------------------------------------------------------------------------------------------------------

router.post('/table', function(req,res) {
    console.log("Requête reçue");
    value = {};
    var busboy = new Busboy({headers: req.headers});
    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
        file.on('data', function(data) {
            value[fieldname] = data.toString('ascii');
        });
    });
    busboy.on('finish', function () {
        sendDataToInfluxDB("connectedtables", value);
        res.writeHead(303, {Connection: 'close', Location: '/'});
        res.end();
    });
    req.pipe(busboy);
});

/**
*   AIR QUALITY DATA (OLD / DEPRECATED)
**/
router.post('/airquality', function(req,res) {
    console.log("Requête reçue");
    value = {};
    var busboy = new Busboy({headers: req.headers});
    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
        file.on('data', function(data) {
            value[fieldname] = data.toString('ascii');
        });
    });
    busboy.on('finish', function () {
        sendDataToInfluxDB("airquality", value);
        res.writeHead(303, {Connection: 'close', Location: '/'});
        res.end();
    });
    req.pipe(busboy);
});

router.post('/sonometer', function(req,res) {
    console.log("Requête reçue");
    value = {};
    var busboy = new Busboy({headers: req.headers});
    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
        file.on('data', function(data) {
            value[fieldname] = data.toString('ascii');
        });
    });
    busboy.on('finish', function () {
        sendDataToInfluxDB("airquality", value);
        res.writeHead(303, {Connection: 'close', Location: '/'});
        res.end();
    });
    req.pipe(busboy);
});

/**
*   AIR QUALITY DATA (JSON)
**/
router.post('/airQualityJSON', function (req,res){
    var data = req.body;
    for(var i=0; i<data.length; i++){
        sendDataToInfluxDB("qarpediem", data[i]);
    }
    res.status(200).end();
});

/**
*   SONOMETER DATA (JSON)
**/
router.post('/sonometerJSON', function (req,res){
    var data = req.body;
    for(var i=0; i<data.length; i++){
        sendDataToInfluxDB("qarpediem", data[i]);
    }
    res.status(200).end();
});

function sendDataToInfluxDB(database, data){
    key = Object.keys(data);
    console.log(key);
    connectToInfluxDB(database);
    var date = new Date(data.date);
    console.log(date.toISOString());
    for(var i=0; i<key.length; i++){
        if(data[key[i]] !== "adress"){
            console.log("Key : " + key[i] + "; Value : " + data[key[i]]);
            influxDB.writePoints([
                {
                    measurement : key[i],
                    tags : { device : data.address },
                    fields : { value : data[key[i]]},
                    timestamp: date // in milliseconds
                }
            ]).catch(function(e) {
                console.log("ERROR");
                console.log(e);
            });}}}

function getTheLastTimeInTheData(data){
    var date = moment("1970-01-01T00:00:00.000");
    for(var i=0; i<data.length; i++){
        if(moment(data[i].time).isAfter(date)) date = moment(data[i].time);
    }
    return date.format("dddd, MMMM Do YYYY, h:mm:ss a");
}

router.get('/liveTableAvailabilityView', function (req,res){
    var data = {};
    connectToInfluxDB('connectedtables');
    influxDB.query("SHOW MEASUREMENTS").then(function(results) {
        if (results.length === 0) {
            res.render('noLiveDataFound');
        }
        else {
            var measurement = "";
            for (var i = 0; i < results.length; i++) {
                if (results[i].name.includes("chair")) {
                    if ((measurement === "")) {
                        measurement = measurement + results[i].name;
                    }
                    else {
                        measurement = measurement + ", " + results[i].name;
                    }
                }
            }
        }
        influxDB.query("SELECT last(value) FROM " + measurement).then(
            function (results) {
                data.date = getTheLastTimeInTheData(results);
                data.values = {};
                var j = 0;
                for (var i = 0; i < results.groupRows.length; i++) {
                    //data[results.groupRows[i].name] = results.groupRows[i].rows[0].last; //Après implémentation valeurs : 0,1,2
                    // if(results.groupRows[i].rows[0].last === 0) data[results.groupRows[i].name] = "vide";
                    // else if(results.groupRows[i].rows[0].last === 1) data[results.groupRows[i].name] = "chaise";
                    // else data[results.groupRows[i].name] = "personne";
                    data.values[results.groupRows[i].name] = results.groupRows[i].rows[0].last;
                    j++;
                }
                data.nbChair = j;
                console.log(data);
                res.render('liveTableAvailability', data);
            });
    }
    )}
);

//----------------------------------------------------------------------------------------------------------------------
//---------------------- REQUÊTES POUR ACCÈS AUX DERNIÈRES DONNÉES ENREGISTRÉES (DITES "LIVE") -------------------------
//----------------------------------------------------------------------------------------------------------------------

router.get('/liveConnectedTable', function(req,res){throwLiveData(req,res)});

router.get('/liveAirQuality', function(req,res){throwLiveData(req,res)});

router.get('/liveSonometer', function(req,res){throwLiveData(req,res)});

router.get('/refreshLiveSonometer', function(req,res){refreshLiveData(req,res)});

router.get('/refreshLiveConnectedTable', function(req,res){refreshLiveData(req,res)});

router.get('/refreshLiveConnectedTable', function(req,res){refreshLiveData(req,res)});

router.get('/liveTotemView', function(req, res, next){
    connectToInfluxDB('totem');
    var data = {
        title : 'Totem'
    };
    res.render('liveTotemView', data);
});

function throwLiveData(req, res){
    var data = {};
    data.measurements = [];
    data.data = [];
    if(req.url === '/liveConnectedTable'){
        connectToInfluxDB('connectedtables');
        link = 'liveConnectedTable';
    }
    else if(req.url === '/liveAirQuality'){
        connectToInfluxDB('airquality');
        link = 'liveAirQuality';
    }
    else if(req.url === '/liveSonometer'){
        connectToInfluxDB('airquality');
        link = 'liveSonometer';
    }
    data.url = link;
    influxDB.query("SHOW MEASUREMENTS").then(function(results){
        console.log(results);
        if(results.length === 0){
            res.render('noLiveDataFound');
        }
        else {
            data.measurements[0] = results[0].name;
            var measurements = results[0].name;
            for(var i=1; i<results.length; i++){
                if(results[i].name){
                    measurements = measurements + ", " + results[i].name;
                    data.measurements[i] = results[i].name;
                }
            }
            influxDB.query("SELECT last(value) FROM " + measurements).then(
                function (results) {
                    data.currDate = getTheLastTimeInTheData(results);
                    for(var i=0; i<results.groupRows.length; i++){
                        data.data[i] = results.groupRows[i].rows[0].last;
                    }
                    res.render(link,data);
                })}})}

function refreshLiveData(req,res){
    console.log(req.query);
    var callback = req.query.callback;
    var data = {};
    data.measurements = [];
    data.data = {};

    influxDB.query("SHOW MEASUREMENTS").then(function(results){
        if(results.length === 0){
            res.render('noLiveDataFound');
        }
        else {
            data.measurements[0] = results[0].name;
            var measurements = results[0].name;
            for(var i=1; i<results.length; i++){
                if(results[i].name){
                    measurements = measurements + ", " + results[i].name;
                    data.measurements[i] = results[i].name;
                }
            }
            influxDB.query("SELECT last(value) FROM " + measurements).then(
                function (results) {
                    data.currDate = getTheLastTimeInTheData(results);
                    for(var i=0; i<results.groupRows.length; i++){ data.data[results.groupRows[i].name] = results.groupRows[i].rows[0].last; }
                    res.status(200).send(callback + '(' + JSON.stringify(data) + ');');
                })}})}

router.get('/lastValues', function(req,res){

    var sensorId = req.query.sensorId;
    var data = {};
    if((sensorId)<1000){
        influxDB.query("SELECT last(value) FROM temperature, humidity, dewPoint, boardTemp WHERE device='"+sensorId+"'").then(
            function (results){
                data.date = getCorrectDateFormat(results[0].time);
                data.temperature = getSpecificDataFromMultipleSelectQuery("temperature",results);
                data.humidity = getSpecificDataFromMultipleSelectQuery("humidity", results);
                data.dewPoint = getSpecificDataFromMultipleSelectQuery("dewPoint", results);
                data.boardTemp = getSpecificDataFromMultipleSelectQuery("boardTemp", results);
                res.status(200).send(""+ req.query.callbackFunction + '(' + JSON.stringify(data) + ',"'+req.query.sensorId+'");');});}
    else{
        influxDB.query("SELECT last(value) FROM temperature, sensorRaw, delta, vBat WHERE device='"+sensorId+"'").then(
            function(results){
                data.date = getCorrectDateFormat(results[0].time);
                data.temperature = getSpecificDataFromMultipleSelectQuery("temperature",results);
                data.sensorRaw = getSpecificDataFromMultipleSelectQuery("sensorRaw", results);
                data.delta = getSpecificDataFromMultipleSelectQuery("delta", results);
                data.vBat = getSpecificDataFromMultipleSelectQuery("vBat", results);
                res.status(200).send(""+ req.query.callbackFunction + '(' + JSON.stringify(data) + ',"'+req.query.sensorId+'");');});}});

router.get('/lastNValues', function(req,res){
    var sensorId = req.query.sensorId;
    var num = req.query.n;
    var data = [];
    console.log(req.query);
    if((sensorId)<1000){
        data.push(["date","temperature" ,"humidity" ,"dewPoint","boardTemp"]);
        influxDB.query("SELECT value FROM temperature, humidity, dewPoint, boardTemp WHERE device='"+sensorId+"' LIMIT " + num).then(
            function (results){
                for( var i = 0 ; i<results.groupRows[0].rows.length; i++){
                    date = getCorrectDateFormat(results.groupRows[0].rows[i].time);
                    temperature = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("temperature", results, i);
                    humidity = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("humidity", results, i);
                    dewPoint = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("dewPoint", results, i);
                    boardTemp = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("boardTemp", results, i);
                    data.push([date,temperature,humidity,dewPoint,boardTemp]);}
                res.status(200).send(""+ req.query.callbackFunction + '(' + JSON.stringify(data) + ',"'+req.query.sensorId+'");');});}
    else{
        data.push(["date" , "temperature" , "sensorRaw" , "delta" , "vBat"]);
        console.log("SELECT value FROM temperature, sensorRaw, delta, vBat WHERE device='"+sensorId+"' LIMIT " + num);
        influxDB.query("SELECT value FROM temperature, sensorRaw, delta, vBat WHERE device='"+sensorId+"' LIMIT " + num).then(
            function(results){
                console.log(results.groupRows[0].rows[0].time);
                for( var i = 0 ; i<results.groupRows[0].rows.length; i++){
                    date = getCorrectDateFormat(results.groupRows[0].rows[i].time);
                    temperature = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("temperature", results, i);
                    sensorRaw = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("sensorRaw", results, i);
                    delta = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("delta", results, i);
                    vBat = getSpecificDataFromMultipleSelectQueryAndMultipleLineResult("vBat", results, i);
                    data.push([date,temperature,sensorRaw,delta,vBat]);}
                res.status(200).send(""+ req.query.callbackFunction + '(' + JSON.stringify(data) + ',"'+req.query.sensorId+'");');});}});

function getSpecificDataFromMultipleSelectQuery(variable, data){
    for(var i=0; i<data.groupRows.length; i++) if(data.groupRows[i].name === variable) return data[i].last;
}

function getSpecificDataFromMultipleSelectQueryAndMultipleLineResult(variable, data, n){
    for(var i=0; i<data.groupRows.length; i++) if(data.groupRows[i].name === variable) return data.groupRows[i].rows[n].value;
}

function getCorrectDateFormat(date){
    date = moment(date).format();
    return  date.substring(0,10) + " " + date.substring(11,19);}

//Récupération de l'heure

router.get('/currentTime', function(req,res){
    res.send(Date.now());
});


module.exports = router;