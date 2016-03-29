var socketURI = "ws://"+window.location.host+"/websocket/ws.cgi"
var status = 0;
var numColors = 1;
var bright = 50;
var speed = 50;
var fx = 0;
var queue = ["000000", "000000", "000000", "000000", "000000"];  

function init(){
    openWebSocket();    //do what you need here
}

function openWebSocket(){
    websocket = new WebSocket(socketURI);
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { receive(evt) };
    websocket.onerror = function(evt) { onError(evt) };
}

function connect(){
    openWebSocket();
}

function onOpen(evt){
    document.getElementById('status').innerHTML = "CONNECTED";
    sendUpdate();
}

function onClose(evt){
    document.getElementById('status').innerHTML = "DISCONNECTED";
}

function onError(evt){
    document.getElementById('status').innerHTML = "ERROR";
}

function receive(evt){
    document.getElementById('message').innerHTML = evt.data;
}

function send(msg){
    websocket.send(msg);
    document.getElementById('message').innerHTML = "SENT: " + msg;
}

function shift(){
    for (i=4; i>0; i--){
	queue[i] = queue[i-1];
    }
}

function updateQueue(){
    send('NCOLORS=' + numColors.toString() + ';');
    send('COLORS=' 
	 + queue[0] + ',' 
	 + queue[1] + ','
	 + queue[2] + ','
	 + queue[3] + ','
	 + queue[4] + ';');
}

function updateFX(){
    send('FX=' + fx.toString() + ';');
}

function updateSpeed(){
    send('SPEED=' + speed.toString() + ';');
}

function updateBrightness(){
    send('BRIGHT=' + bright.toString() + ';');
}

function updateStatus(){
    send('SWITCH=' + status.toString() + ';');
}

function sendUpdate(){
    updateStatus();
    updateQueue();
    updateFX();
    updateSpeed();
    updateBrightness();
}

function showQueue(){
    for (i=0; i<numColors; i++){
	cell = qcell(i);
	cell.style.backgroundColor = queue[i];
    }
    for (i=numColors; i<5; i++){
	cell = qcell(i);
	cell.style.backgroundColor = "#404040";
    } 
}

function col(c) {
    shift();
    queue[0] = c;
    showQueue();
    updateQueue();
}

function num(n) {
    numColors = n;
    showQueue();
    updateQueue();
}

function inc() {
    if (numColors < 5){
	numColors = numColors+1;
	showQueue();
	updateQueue();
    }
}

function dec() {
    if (numColors > 1){
	numColors = numColors-1;
	showQueue();
	updateQueue();
    }
}

function onoff() {
    but = document.getElementById('switch');
    if (status == 0) {
	but.style.background = "#a0a0a0";
	status = 1;
    } else {
	but.style.background = "#000000";
	status = 0;
    }		
    updateStatus();
}

function setFX(f) {
    if (f==0) {
	document.getElementById('fade').disabled = true;
	document.getElementById('fade').style.background = "#a0a0a0";
	document.getElementById('wheel').disabled = false;
	document.getElementById('wheel').style.background = "#000000";
	fx = 0;
    } else if (f==1) {
	document.getElementById('fade').disabled = false;
	document.getElementById('fade').style.background = "#000000";
	document.getElementById('wheel').disabled = true;
	document.getElementById('wheel').style.background = "#a0a0a0";
	fx = 1;
    }
    updateFX();
}

function setBrightness(v) {
    bright = v;
    updateBrightness();
}

function setSpeed(v) {
    speed = v;
    updateSpeed();
}

function qcell(i) {
    if (i == 0){
	return document.getElementById('q1');  
    } else if (i == 1){
	return document.getElementById('q2');  
    } else if (i == 2){
	return document.getElementById('q3');  
    } else if (i == 3){
	return document.getElementById('q4');  
    } else if (i == 4){
	return document.getElementById('q5');  
    }
}

window.addEventListener("load", init, false);
