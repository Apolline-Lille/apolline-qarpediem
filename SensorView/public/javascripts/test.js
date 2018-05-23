function getUrlVars() {
    var vars = {};
    var parts = document.currentScript.src.replace(/[?&]+([^=&]+)=([^&]*)/gi,
        function(m,key,value) {
            vars[key] = value;
        });
    return vars;
}

var params = getUrlVars();
console.log(document.currentScript.src);
console.log(params);
var callback = params.callbackFunction;
var action = params.action;

console.log(action);
var data = 789;

//window[callback](data);