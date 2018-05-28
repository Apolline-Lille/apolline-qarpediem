"use strict";

/**
 * QARPEDIEM Private namespace
 **/
(function(){

let sqlite_request = require("./sqlite");
let { ipcRenderer } = require("electron");
let ChartJS = require("chart.js");
let moment = require("moment");

let loading_count = 0;

let chart = new ChartJS(document.querySelector("#sensors_plot"), {
	type: "line",
	data: {
		datasets: [],
	},
	options: {
		scales: {
			xAxes: [{
				type: "time",
				time: {
					unit: "second",
					displayFormats: {
						quarted: "LTS"
					}
				},
				ticks:{
					autoSkip: true
				}
			}],
		},

		animation: {
			duration: 0
		}
	}
});

setInterval(function(){
	if(loading_count === 0){
		document.querySelector("#loading img").style.display = "none";
	} else {
		document.querySelector("#loading img").style.display = "inline-block";
	}
}, 10);

loading_count++;
let sql_get_sensor_list = "SELECT identifier,common_name,unit,chart_color FROM config.sensors;"
sqlite_request(sql_get_sensor_list, function(data){
	for(let i = 0; i < data.length; i ++){
		add_sensor(data[i]);
	}
	loading_count--;
});

function add_sensor([identifier, display_name, unit, chart_color]){
	let sensor = document.createElement("a");
	sensor.href = "#";
	sensor.classList.add("list-group-item");
	sensor.dataset.identifier = identifier;
	sensor.dataset.display_name = display_name;
	sensor.dataset.unit = unit;
	sensor.dataset.chart_color = chart_color;
	sensor.innerHTML = display_name;

	document.querySelector("#sensors_list").appendChild(sensor);

	sensor.addEventListener("click", toggle_sensor);
}

function manage_loading(){
	if(loading_count === 0){
		document.querySelector("#loading img").style.display = "none";
	} else {
		document.querySelector("#loading img").style.display = "inline";
	}
}

function toggle_sensor(event){
	let element = event.target;
	let sensor = element.dataset.identifier;

	loading_count++;
	if(event.target.classList.contains("active")){
		disable_sensor(sensor, element);
	} else {
		enable_sensor(sensor, element);
	}
}

function disable_sensor(sensor, elem){
	elem.classList.remove("active");
	for(let i = 0; i < chart.data.datasets.length; i++){
		if(chart.data.datasets[i].sensor_identifier == sensor){
			chart.data.datasets.splice(i, 1);
			break;
		}
	}
	chart.update();
	loading_count--;
}

function enable_sensor(sensor, elem){
	elem.classList.add("active");

	let dataset = {
		label: elem.dataset.display_name,
		data: [],
		backgroundColor: elem.dataset.chart_color,
		borderWidth: 1,
		type: "scatter",
		sensor_identifier: sensor,
		borderColor: elem.dataset.chart_color,
		spanGaps: true,
		pointHoverBackgroundColor: elem.dataset.chart_color,
		fill: false,
		showLine: false,
		pointRadius: 1,
		scales: {
			yAxes: {
			}
		}
	};

	elem.chart_dataset = dataset;
	chart.data.datasets.push(dataset);
	chart.update();

	update_sensor(sensor, elem);
}

function update_sensor(sensor, elem){
	let req =
	"SELECT data,poll_time FROM sensors.data WHERE sensor_identifier='" + sensor + "';";
	let sonometer_req = 
	"SELECT data,poll_time FROM sonometer.sonometer_data WHERE sensor_identifier='" + sensor + "';";
	let dataset = elem.chart_dataset;

	sqlite_request(req, function(data_rows){
		if(data_rows !== null){
			let data = [];
			let time = [];
			dataset.data = [];
			data_rows.forEach((row) => {
				dataset.data.push({
					x: moment.unix(Number(row[1])),
					y: Number(row[0])
				});
			});

			for(let i = 0; i < time.length; i++){
				if(chart.data.labels.indexOf(time[i]) == -1){
					dataset.data.push(data[i]);
					chart.data.labels.push(time[i]);
				} else {
					dataset.data.push(null);
				}
			}

			chart.update();
		}

		loading_count--;
	});

	sqlite_request(sonometer_req, function(data_rows){
		if(data_rows !== null){
			let data = [];
			let time = [];
			dataset.data = [];
			data_rows.forEach((row) => {
				dataset.data.push({
					x: moment.unix(Number(row[1])),
					y: Number(row[0])
				});
			});

			for(let i = 0; i < time.length; i++){
				if(chart.data.labels.indexOf(time[i]) == -1){
					dataset.data.push(data[i]);
					chart.data.labels.push(time[i]);
				} else {
					dataset.data.push(null);
				}
			}

			chart.update();
		}

		loading_count--;
	});
}

setInterval(() => {
	let elems = document.querySelectorAll("#sensors_list .active");
	loading_count += elems.length;
	console.log("Refreshing " + elems.length + " sensors");

	elems.forEach((elem) => {
		console.log("Refreshing " + elem.dataset.identifier);
		update_sensor(elem.dataset.identifier, elem);
	});
}, 10000);

})();
