
/*--------------------------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *--------------------------------------------------------------------------------------------*/
var websocket;


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

    var today = new Date();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();

    console.log("Client's time: " + time);

    // Send message to the server
    sendMessage("time", time);
}

function onClose(event) {
    // If for some reason connection is closed
    console.log('Connection closed');
    // Set timeout 2s - try to connect to ws again
    setTimeout(initWebSocket, 2000);
}

function receiveMessage(event) {
    // Convert from string to JSON
    let data = JSON.parse(event.data);

    //console.log(data);

    switch(data.action) {
        case  "power":
            var state;
            $("#power").text(data.value);
            break;

        case "pitch":
            $("#pitch").text(data.value);
            break;

        case "roll":
            $("#roll").text(data.value);
            break;
    }    
}

// {"action":"ledstate", "value":"1"}
function sendMessage(action, value) {
    websocket.send(JSON.stringify({"action":action, "value": value}));
}


/*--------------------------------------------------------------------------------------------
 * BUTTONS
 *--------------------------------------------------------------------------------------------*/
function turnServerOff(){
    console.log("Turn the server off!");
    sendMessage("off", 0);
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