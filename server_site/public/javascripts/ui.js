"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var probeTimeout0;
var probeTimeout1;
var errorTimeout;
var resetErrorTimeout;
var numStalls=[0,0];
var connectionError = false;
var showTime = 800;
var hideTime = 500;
var runningProgram = 0;
var stoppedProgram = 1;


var adminMode= false;

var socket = io.connect();
$(document).ready(function() {
	setReady("0");
	setReady("1");
	displayRunningWashroom();
	$('#toggle').click(function(){
		if (runningProgram==1){
			runningProgram=0;
			stoppedProgram =1;
		}else {
			runningProgram = 1;
			stoppedProgram = 0;
			
		}
		displayRunningWashroom();
	});
	$('#admin').click(function(){
		if($('#admin').text() == "Admin"){
			var pass = prompt("Enter the 4 digit password");
			if (pass == null){
				//do nothing, the user cancelled the prompt
			} else if (!isNaN(pass)){
				console.log(pass);
				//hardcode only sending admin password to washroom 0
				sendCommand(0,"adminPass " + pass);
			} else if (pass == "" || isNaN(pass)){
				$("#error-box-bad-pass").show(showTime);
				hideBox("#error-box-bad-pass", 5000);
			}
		} else {
			disableAdminMode();
			displayRunningWashroom();
		}
	
	});

	probe(runningProgram);


	socket.on('bberror', function(result) {
		var msg = 'SERVER ERROR: ' + result;
		$('#error-text').text(msg);
		$('#error-box').show(showTime);
		connectionError=true;
		$('.disableOnError').prop('disabled', true);

		resetErrorStatus();
	});

	socket.on('requestRecieved', function(result){
		clearTimeout(errorTimeout);
	});
	
	socket.on('commandReply', function(result) {


		var infoInit = result.split("_");
		var info = infoInit.slice(1, infoInit.length);
		handleReplies(infoInit[0], info);
	});
	
});


function probe(){
	var bathroomNum = runningProgram
	if(bathroomNum == 0){
		probeTimeout0= setTimeout(function(){
			sendCommand(bathroomNum, "getOpenStalls");
			sendCommand(bathroomNum, "getTotalStalls");
			if(adminMode){
				sendCommand(bathroomNum, "getMaintenance");
			}
			checkNodeStatus();

			probe();

		},1000);
	} else {
		probeTimeout1= setTimeout(function(){
			sendCommand(bathroomNum, "getOpenStalls");
			sendCommand(bathroomNum, "getTotalStalls");
			if(adminMode){
				sendCommand(bathroomNum, "getMaintenance");
			}
			checkNodeStatus();

			probe();

		},1000);
	}

}

function sendRelease(bathroomNum, num){
	sendCommand(bathroomNum, "serviceRel " + num.toString());
}

function checkNodeStatus(){
	errorTimeout = setTimeout(function() {
		var msg = 'SERVER ERROR: No response from NodeJS server. Is the server running?';
		$('#error-text').text(msg);
		$('#error-box').show(showTime);
		connectionError = true;
		$('.disableOnError').prop('disabled', true);
		
	}, 1000);
}

function resetErrorStatus(){
	clearTimeout(resetErrorTimeout);
	resetErrorTimeout = setTimeout(function() {
		$('#error-box').hide(hideTime);
		$("#error-box-bad-pass").hide(hideTime);
		connectionError = false;
		$('.disableOnError').prop('disabled', false);
		$(".active"+stoppedProgram).prop('disabled', true);
	}, 3000);
}

function sendCommand(bathroomNum, message) {

	socket.emit('request',bathroomNum+" "+message);
};

function hideBox(boxName, time){
	var boxTimeout;
	boxTimeout = setTimeout(function(){
		$(boxName).hide(hideTime);
	}, time);
}

function enableAdminMode(){
	adminMode = true;
	$("body").css("background-color", "#045");
	$('.whiteIfAdmin').css('color', 'white');
	$('#admin').text("Return to User View");
	$('.onlyAdminMode').show(showTime);
	$('.disableOnAdmin').prop('disabled', true);

}

function disableAdminMode(){
	adminMode=false;
	$("body").css("background-color", "white");
	$('.whiteIfAdmin').css('color', 'black');
	$('#admin').text("Admin");
	$('.disableOnAdmin').prop('disabled', false);
	$('.onlyAdminMode').hide(hideTime);

}

function setReady(sNum){
	$('#stats'+ sNum).click(function(){
		sendCommand(sNum, " stats");
		showStats();
	});
	$('#reserve'+ sNum).click(function(){
		sendCommand(sNum, " reserve");
	});
	$('#service'+ sNum).click(function(){
		for(var i=0; i<numStalls[sNum];i++){
		 	$('#chooseStall'+sNum).append("<option>" +i.toString()+ "</option>");
		}
		$('#chooseStallForm' + sNum).show(showTime);
	});

	$('#sendServiceRequest'+ sNum).click(function(){
		var selected = $('#chooseStall' + sNum).find(":selected").text()
		sendCommand(sNum, "serviceReq " + selected.toString());
		$('#chooseStall'+ sNum).empty();
		$('#chooseStallForm' + sNum).hide(hideTime);
	});
	
	$('#cancelServiceRequest'+ sNum).click(function(){
		$('#chooseStall' + sNum).empty();
		$('#chooseStallForm'+ sNum).hide(hideTime);
	});
}

function handleReplies(bathroomNum, info){

	if(info[0] == "#success-box"){
		$(info[0]+bathroomNum).show(showTime);
		hideBox(info[0], 5000);
	} else if (info[0] == "#warning-box"){
		$(info[0]).show(showTime);
		hideBox(info[0], 5000);
	}else if (info[0] == "#info-box"){
		$(info[0]).show(showTime);
		hideBox(info[0], 5000);
	} else if (info[0] == "#info-box-service-complete"){
		$(info[0]).show(showTime);
		hideBox(info[0], 5000);
	} else if (info[0]== "#openStalls"){
		$(info[0]+bathroomNum).text(info[1]);
	}else if (info[0]== "#totalStalls"){
		numStalls[bathroomNum] = info[1];
		$(info[0]+bathroomNum).text(info[1]);
		if(!adminMode){
			$(".blockIfNotLoaded"+runningProgram).prop('disabled', false);
		}
	} else if (info[0] == "#statResult"){
		$(info[0]+bathroomNum).text(info[1]);
		saveStatsData(info[1]);
	} else if (info[0] == "PASSWORD"){
		if(info[1] == "BAD"){
			$("#error-box-bad-pass").show(showTime);
			hideBox("#error-box-bad-pass", 5000);
		} else {
			enableAdminMode();
		}
	} else if (info[0] =="MAINTENANCE"){
		if(info[1]==""||info[1]==null){
			$("#maintenanceHolder"+bathroomNum).empty();
		} else {
			var stalls = info[1].split(" ");
			$("#maintenanceHolder" + bathroomNum).empty();
			for(var i=0; i<stalls.length;i++){
				if(stalls[i]!=''&& !isNaN(stalls[i]) && stalls[i]< numStalls[bathroomNum]){
					$("#maintenanceHolder" + bathroomNum).append('<button type="button" onclick="sendRelease(' +bathroomNum + ", " +stalls[i] +')" id="stall'+ stalls[i]+'" class="releaseStall btn btn-lg btn-block btn-success disableOnError"> Mark Stall '+stalls[i]+ ' as serviced.</button>');
				}
			}
		}
	}
}

function displayRunningWashroom(){
	$(".active"+stoppedProgram).prop('disabled', true);
	$(".active"+runningProgram).prop('disabled', false);
	$("#stallContainer"+runningProgram).css('color', '#000');
	$("#stallContainer"+stoppedProgram).css('color', '#BBB');
}