/**
 * Created by carbon on 06/06/2017.
 */
console.log("import dateConversionFunctions.js");

function datetimelocalToDateObject(s) {
    var b = s.split(/\D+/);
    return new Date(b[0], --b[1], b[2], b[3], b[4], b[5]||0, b[6]||0);
}

function dateToDateTimeLocal(d) {
    // Default to now if no date provided
    d = d || new Date();
    // Pad to two digits with leading zeros
    function pad(n){
        return (n<10?'0':'') + n;
    }
    // Pad to three digits with leading zeros
    function padd(n){
        return (n<100? '0' : '') + pad(n);
    }
    // Convert offset in mintues to +/-HHMM
    // Note change of sign
    // e.g. -600 => +1000, +330 => -0530
    function minsToHHMM(n){
        var sign = n<0? '-' : '+';
        n = Math.abs(n);
        var hh = pad(n/60 |0);
        var mm = pad(n%60);
        return  sign + hh + mm;
    }
    var offset = minsToHHMM(d.getTimezoneOffset() * -1);
    return d.getFullYear() + '-' + pad(d.getMonth() + 1) + '-' + pad(d.getDate()) +
        'T' + pad(d.getHours()) + ':' + pad(d.getMinutes()) + ':' + pad(d.getSeconds()) +
        '.' + padd(d.getMilliseconds()) + offset;
}