"use strict";

/**
 * QARPEDIEM Private namespace
 **/
(function(){

let sqlite_request = require("./sqlite");
let $ = require("jquery");
require("jquery-deserialize");

let template = $("#sensor_template");
template.remove();

function apply_sensor(e){
	let sensor = $(e.target);
	let id = sensor.data("id");
	let data = sensor.serializeArray();

	for(let {name, value} of data){
		let request = "UPDATE config.sensors SET '";
		request += name;
		request += "'='";
		request += value.replace("'", "\\'");
		request += "' WHERE identifier='";
		request += id;
		request += "';";
	
		sqlite_request(request, function(response){
			console.log(response);
		});
	}


	e.preventDefault();
}

function add_sensor([identifier, display, sensitivity, offset, color]){
	let sensor = template.clone();
	sensor.deserialize({
		identifier: identifier,
		display_name: display,
		sensibility: sensitivity,
		offset: offset,
		chart_color: color
	});
	
	sensor.data("id", identifier);
	sensor.id = null;
	sensor.find("h4").text(identifier);

	sensor.on("submit", apply_sensor);
	$("#sensors").append(sensor);
}

sqlite_request(
	"SELECT identifier, common_name, sensibility, offset, chart_color " +
	"FROM config.sensors;", function(sensors){

	if(sensors == null){
		return alert("Can't connect to the database, please try again");
	}

	for(let sensor of sensors){
		add_sensor(sensor);
	}
});

})();
