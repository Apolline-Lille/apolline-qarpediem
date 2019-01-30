let { spawn } = require("child_process");

function SQLite3_CLI(src){
	this.src = src;
	return this;
}

SQLite3_CLI.prototype.request = function(request, callback){
	let full_request = "PRAGMA foreign_keys=on;";
	full_request += "ATTACH DATABASE '" + this.src + "/config.db' AS config;";
	full_request += "ATTACH DATABASE '" + this.src + "/sensors.db' AS sensors;";	
	full_request += request;

	let sqlite = spawn("sqlite3", [":memory:", full_request]);
	let output = "";

	console.log(full_request);

	sqlite.stdout.setEncoding("utf-8");
	sqlite.stdout.on("data", (data) => {
		output += data;
	});

	sqlite.once("close", (code, signal) => {
		console.log(output.length, code);
		if(code == 0){
			callback(this.parse_output(output));
		} else {
			callback(null);
		}
	});
};

SQLite3_CLI.prototype.parse_output = function(output){
	let rows = output.split('\n');
	for(let i = 0; i < rows.length - 1; i++){
		rows[i] = rows[i].split('|');
	}
	rows.pop();
	return rows;
};

module.exports = SQLite3_CLI;

