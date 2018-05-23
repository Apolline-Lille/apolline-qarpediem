/**
 * Created by carbon on 06/06/2017.
 */
console.log('import chartCreation.js');


//GENERATION DU CHARTJS AVEC LES ELEMENTS SELECTIONNES

function createChart(values){

    console.log(values);
    $('#chart').remove();
    $('#canvasChart').append('<canvas id="chart"><canvas>');
    var ctx = document.getElementById('chart').getContext('2d');
    var labs = [];
    for(var i = 0; i<values.time.length; i++){
        labs[i] = values.time[i]; //datetimelocalToDateObject(values.time[i]).toUTCString();
    }
    var vals = values.values;
    var title = "" + values.measurement + " for " + values.idSensor;
    var chart = new Chart(ctx, {
        // The type of chart we want to create
        type: 'line',

        // The data for our dataset
        data: {
            labels: labs,
            datasets: [{
                label: title,
                borderColor: 'rgb(255, 99, 132)',
                data: vals
            }]
        },

        // Configuration options go here
        options: {
            elements: {
                point:{
                    radius: 0
                },
                line: {
                    tension: 0
                }
            },
            scales: {
                xAxes: [{
                    type: 'time',
                    time: {
                        displayFormats: {
                            quarter: 'MMM YYYY'
                        }
                    }
                }]
            },
            bezierCurve: false,


        }
    });
}