
/*--------------------------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *--------------------------------------------------------------------------------------------*/
var websocket;

const action = {
	UPDATE_MEASUREMENT: 0,
	UPDATE_GRAPH:       1,
	SERVER_MANAGEMENT:  2,
	UPDATE_TIME:        3,
}

var client_connected = 0;


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

    client_connected += 1;

    var today = new Date();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();

    console.log("Client's time: " + time);

    // Send message to the server
    sendMessage(action.UPDATE_TIME, time);
}

function onClose(event) {

    client_connected -= 1;

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

        case action.UPDATE_MEASUREMENT:
            console.log(data.v);

            let measurements = data.v.split("|");

            $("#power").text(measurements[0]);
            $("#pitch").text(measurements[1]);
            $("#roll").text(measurements[2]);
            if(measurements[3] != "0"){
                $("#temp").text(measurements[3] + " &#176 C");
            }
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
 * BUTTONS
 *--------------------------------------------------------------------------------------------*/
function turnServerOff(){
    console.log("Turn the server off!");
    sendMessage(action.SERVER_MANAGEMENT, 0);
    // TODO close the browser
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
 * MAIN
 *--------------------------------------------------------------------------------------------*/
//Call this function when page loads
$(document).ready(function(){
    console.log("Hello there!");

    // Start the function to update the time
    updateClock();

    // Init buttons
    $("#btn_off").click(turnServerOff);

    // Init websockets
    initWebSocket();

});