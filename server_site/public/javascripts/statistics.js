// "use strict";

// $(document).ready(function(){
// var stats;

// function saveStatsData(dataStr) {
// 	var data = dataStr.split(" ");
// 	stats[0].push(data[0]);
// 	stats[1].push(data[1]);
// }

// function buildStatsData() {

// }

// function showStats() {
// 	$('#statResult').html('<canvas id="statsChart"></canvas>');

// 	var ctx = document.getElementById("statsChart").getContext('2d');
// 	var myChart = new Chart(ctx, {
// 	    type: 'bar',
// 	    data: {
// 	        labels: ["Red", "Blue", "Yellow", "Green", "Purple", "Orange"],
// 	        datasets: [{
// 	            label: 'Usage per time slot',
// 	            data: [12, 19, 3, 5, 2, 3],
// 	            backgroundColor: [
// 	                'rgba(255, 99, 132, 0.2)',
// 	                'rgba(54, 162, 235, 0.2)',
// 	                'rgba(255, 206, 86, 0.2)',
// 	                'rgba(75, 192, 192, 0.2)',
// 	                'rgba(153, 102, 255, 0.2)',
// 	                'rgba(255, 159, 64, 0.2)'
// 	            ],
// 	            borderColor: [
// 	                'rgba(255,99,132,1)',
// 	                'rgba(54, 162, 235, 1)',
// 	                'rgba(255, 206, 86, 1)',
// 	                'rgba(75, 192, 192, 1)',
// 	                'rgba(153, 102, 255, 1)',
// 	                'rgba(255, 159, 64, 1)'
// 	            ],
// 	            borderWidth: 1
// 	        },
// 	        {
// 	            label: 'Usage per time slot',
// 	            data: {[12, 19, 3, 5, 2, 3]},
// 	            backgroundColor: [
// 	                'rgba(255, 99, 132, 0.2)',
// 	                'rgba(54, 162, 235, 0.2)',
// 	                'rgba(255, 206, 86, 0.2)',
// 	                'rgba(75, 192, 192, 0.2)',
// 	                'rgba(153, 102, 255, 0.2)',
// 	                'rgba(255, 159, 64, 0.2)'
// 	            ],
// 	            borderColor: [
// 	                'rgba(255,99,132,1)',
// 	                'rgba(54, 162, 235, 1)',
// 	                'rgba(255, 206, 86, 1)',
// 	                'rgba(75, 192, 192, 1)',
// 	                'rgba(153, 102, 255, 1)',
// 	                'rgba(255, 159, 64, 1)'
// 	            ],
// 	            borderWidth: 1
// 	        }]
// 	    },
// 	    options: {
// 	        scales: {
// 	        	xAxes: {
// 	        		stacked: true
// 	        	}
// 	            yAxes: [{
// 	                ticks: {
// 	                    beginAtZero:true
// 	                },
// 	                stacked:true
// 	            }]
// 	        }
// 	    }
// 	});
// }




// });//document ready
