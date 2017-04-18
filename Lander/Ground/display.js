//var dataSocket = new WebSocket("...URL...");
var seriesData = [ [], [], [], [], [], [], [], [], [] ];
var graphs = [ ];
var xAxes = [ ];
var yAxes = [ ];
var ids = ["chart", "chart2", "chart3" /* ... */];
var palette = new Rickshaw.Color.Palette( { scheme: 'classic9' } );

function renderGraph(graph, xAxis, yAxis, plotSet, id){
    var graph = new Rickshaw.Graph( {
            element: document.getElementById(id),
            renderer: 'line',
            series: [ {
                    color: palette.color(),
                    data: plotSet
            } ]
    } );
    graph.render();
    graphs.push(graph);
    xAxis = new Rickshaw.Graph.Axis.Time({
        graph: graph,
        timeFixture: new Rickshaw.Fixtures.Time.Local()
    });
    xAxis.render();
    xAxes.push(xAxis);
    yAxis = new Rickshaw.Graph.Axis.Y({
        graph: graph
    });
    yAxis.render();
    yAxes.push(yAxes);
}

//if you want to send data about direction
//buttons would need to be added
//dataSocket.send("LEFT/RIGHT");

//initialize graphs with initial data
/*
dataSocket.onopen = function (event) {
    for(var i=0; i<Object.keys(data).length; i++){
        seriesData[i] = event.data[i];
        renderGraph(graphs[i], xAxes[i], yAxes[i], seriesData[i]);
    }
};
*/

// update graphs with incoming data
// data points/second known?
/*
dataSocket.onmessage = function(event) {
    var newData = JSON.parse(event.data);
    var time = new Date(msg.date);
    for(var i=0; i<Object.keys(data).length; i++){
        seriesData[i].push(event.data[i]);
        graph[i].update();
    }
};
*/

// TESTING:
var random = new Rickshaw.Fixtures.RandomData(150);

for (var i = 0; i < 150; i++) {
    random.addData(seriesData);
}

console.log(seriesData);

for(var i=0; i<3; i++){
    renderGraph(graphs[i], xAxes[i], yAxes[i], seriesData[i], ids[i]);
}

setInterval( function() {
    random.addData(seriesData);
    for(var i=0; i<3; i++){
        graphs[i].update();
    }
}, 1000 );

