const { ipcRenderer } = require("electron");

let req_list = {};

ipcRenderer.on("db_response", function(event, {id, data, sql}){
	let callback;
	console.log("[renderer] db_response id=" + id);
	if(req_list[id] && req_list[id].sql == sql){
		req_list[id].callback(data);
		delete req_list[id];
	}
});

function request(sql, callback){
	let id = 0;
	while((typeof req_list[id]) != "undefined"){
		id++;
	}

	req_list[id] = {
		callback: callback,
		sql: sql
	};

	ipcRenderer.send("db_request", {
		id: id,
		sql: sql
	});
}

module.exports = request;
