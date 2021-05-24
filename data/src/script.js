

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    initButton();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onReceiveMessage;
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

function onReceiveMessage(event) {
    // Convert from string to JSON
    let data = JSON.parse(event.data);

    //console.log(data);

    var key = Object.keys(data);

    switch(data.action) {
        case  "ledstate":
            var state;
            if (data.value == "1"){
                state = "ON";
            }
            else{
                state = "OFF";
            }
            document.getElementById('state').innerHTML = state;
            break;
    
        case "aval":
            document.getElementById("a_val").innerHTML = data.value;
            break;
    }    
}

// {"action":"ledstate", "value":"1"}
function sendMessage(action, value) {
    websocket.send(JSON.stringify({"action":action, "value": value}));
}



function initButton() {
    // When button is clicked, function toggle is called
    document.getElementById('button').addEventListener('click', toggleLed);
}

function toggleLed(){
    
    let state = document.getElementById('state').innerHTML;
    var val = 0;

    if(state == "ON"){
        val = 0;
    } else {
        val = 1;
    }

    sendMessage("ledstate", val);
}