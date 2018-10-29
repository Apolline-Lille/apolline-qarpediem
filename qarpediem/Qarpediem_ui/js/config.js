"use strict";

/**
 * QARPEDIEM Private namespace
 **/
(function(){

let sqlite_request = require("./sqlite");
let $ = require("jquery");
require("jquery-deserialize");

function save_and_apply_config(e){
	e.preventDefault();

	let settings = $("#config_form").serializeArray();

	let request = "BEGIN IMMEDIATE;\n";
	for(let { name, value } of settings){
		request += "UPDATE config.settings SET value='";
		request += value.replace("'", "\\'");
		request += "' WHERE name='";
		request += name.replace("'", "\\'");
		request += "';\n";
	}

	request += "COMMIT;";

	console.log(request);

	sqlite_request(request, () => {
		alert("Configuration saved");
	});
}

sqlite_request("SELECT name, value FROM config.settings", function(settings_raw){
	if(settings_raw == null){
		return alert("Could not connect to database, please try again");
	}

	let settings = {};
	for(let [name, value] of settings_raw){
		settings[name] = value;
	}

	$("#config_form").deserialize(settings);
	$("#config_form").on("submit", save_and_apply_config);
});

})();
