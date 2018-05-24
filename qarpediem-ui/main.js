const { app, BrowserWindow, ipcMain } = require("electron");
const path = require("path");
const url = require("url");
const sqlite_cli = require("./sqlite_cli");

let main_window = null;

app.on("ready", () => {
	let db = new sqlite_cli("/home/qarpediem");
  	main_window = new BrowserWindow({
		width: 900,
		height: 700,
		show: false
 	});

	main_window.maximize();

	main_window.loadURL(url.format({
		pathname: path.join(__dirname, "index.html"),
		protocol: 'file:',
		slashes: true,
	}));

	main_window.on("closed", () => {
		main_window = null;
		process.exit(0);
	});

	main_window.on("window-all-closed", () => {
        	process.exit(0);
	});

	main_window.on("ready-to-show", function(){
		main_window.show();
	});

	ipcMain.on("db_request", function(event, request){
		console.log("[main_process] db_request id=" + request.id);
		db.request(request.sql, function(data){
			console.log("[main_process] db_response id=" + request.id);

			event.sender.send("db_response", {
				id: request.id,
				data: data,
				sql: request.sql
			});
		});
	});
});

