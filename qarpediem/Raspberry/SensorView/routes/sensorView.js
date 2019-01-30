/**
 * Created by carbon on 04/07/2017.
 */
var express = require('express');
var router = express.Router();
var moment = require('moment');

//INSTANCIATION DE L'OBJET INFLUXDB
var influx = require('influx');
var influxDB = null;


module.exports = router;