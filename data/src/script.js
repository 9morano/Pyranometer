
/*--------------------------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *--------------------------------------------------------------------------------------------*/
var websocket;

const action = {
    REAL_TIME_DATA:     0,
    SERVER_OFF:         1,
    SERVER_TIME:        2,
    DELETE_MEASUREMENT: 3,
    START_MEASUREMENT:  4,
    UPDATE_MEASUREMENT: 5,
}


/*--------------------------------------------------------------------------------------------
 * WEB SOCKETS 
 *--------------------------------------------------------------------------------------------*/
function initWebSocket() {
    var gateway = `ws://${window.location.hostname}/ws`;
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = receiveMessage;
}

function onOpen(event) {
    console.log('Connection opened');

    if(document.URL.includes("index")){
        var today = new Date();
        var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
        sendMessage(action.SERVER_TIME, time);
    }
    else{
        // Action with value 1 updates list of stored measurement files
        deleteAllMeasurementsFile();
        sendMessage(action.UPDATE_MEASUREMENT, "0");
    }
}

function onClose(event) {

    // If for some reason connection is closed
    console.log('Connection closed');
    // Set timeout 2s - try to connect to ws again
    setTimeout(initWebSocket, 2000);
}

function receiveMessage(event) {

    let data = JSON.parse(event.data);

    //console.log(data);
    // JSON object is formed with action (a) and value (v)
    // Values for measurements are separated by char |
    // Example {"a":"p", "v":"1|2|3|4"}

    switch(data.a) {

        case action.REAL_TIME_DATA:
            console.log(data.v);

            let measurements = data.v.split("|");

            $("#power").text(measurements[0]);
            $("#pitch").text(measurements[1]);
            $("#roll").text(measurements[2]);
            if(measurements[3] != "0"){
                $("#temp").text(measurements[3] + " &#176 C");
            }
            break;

        case action.UPDATE_MEASUREMENT:
            console.log("New measurement file" + data.v);
            addMeasurementFile(data.v);
            break;

        case action.START_MEASUREMENT:
            $("#msmnt_0").text(data.v);
            break;

        default:
            console.log("Undefined action" + data.a);
            break;
    }    
}

// {"a":"p", "v":"1"}
function sendMessage(action, value) {
    websocket.send(JSON.stringify({"a":action, "v": value}));
}


/*--------------------------------------------------------------------------------------------
 * BUTTON - SERVER OFF
 *--------------------------------------------------------------------------------------------*/
function turnServerOff(){
    console.log("Turn the server off!");
    sendMessage(action.SERVER_OFF, 0);

    // TODO close the browser - doesn't work with Firefox 86 
    var conf = confirm("Če se okno ne zapre, ga zaprite ročno!");
    if(conf){
        let window = open(location,'_self');
        window.close();
        return false;
    }
}


/*--------------------------------------------------------------------------------------------
 * TIME UPDATE
 *--------------------------------------------------------------------------------------------*/
// Function to update time on HTML site periodically every second
function updateClock(){
    var now = new Date();
    var time = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
    $("#time").text(time);
    setTimeout(updateClock, 1000);
}



/*--------------------------------------------------------------------------------------------
 * MEASUREMENTS
 *--------------------------------------------------------------------------------------------*/
function addMeasurementFile(filename){

    var del_btn = $('<button class="btn btn-secondary float-right" style="margin-right: 2px;"/>').text("Izbriši").click(function(){
        deleteMeasurementFile(filename);
    });
    var dl_btn  = $('<button class="btn btn-secondary float-right"/>').text("Prenesi").click(function(e){
        e.preventDefault();
        downloadMeasurementsFile(filename);
    });
    var m = $('<p id="' + filename + '"><b>Meritev: </b>' + filename + '</p>');

    m.append(dl_btn).append(del_btn);

    $("#stored_measurements").after(m);
}

function deleteMeasurementFile(filename){
    console.log("Izbrisi " + filename);

    document.getElementById(filename).remove();
    sendMessage(action.DELETE_MEASUREMENT, filename);
}

function deleteAllMeasurementsFile(){
    document.getElementById("measurements_container").remove();

    var element = $('<div id="measurements_container"> <h2 id="stored_measurements">SHRANJENE MERITVE</h2></div>');
    $("#neki").after(element);
}

function downloadMeasurementsFile(filename){
    console.log("Prenesi " + filename);
    window.location.href = "/download?file=" + filename;
}

function startNewMeasurement(){
    let filename = $("#msmnt_name").val();

    if(filename.length > 12){
        window.alert("Dolžina imena je lahko max 12 znakov :(");
        return;
    }

    let period = $("#perioda_sek").text();

    var data =  filename + "|" + period;
    console.log(data);
    sendMessage(action.START_MEASUREMENT, data);
}

/*--------------------------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------------------------*/
//Call this function when page loads
$(document).ready(function(){

    // Init buttons
    $("#btn_off").click(turnServerOff);

    if(document.URL.includes("index")){
        console.log("Hello there!");

        // Start the function to update the time
        updateClock();
    }
    else{
        console.log("MSMNTS");
        $("#izvedi").click(startNewMeasurement);

        // Update slider seconds val
        document.getElementById("period_range").oninput = function() {
            $("#perioda_sek").text(this.value);
        }
    }

    // Init websockets
    initWebSocket();
});