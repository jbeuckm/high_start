var svg = d3.select("svg"),
    margin = {
        top: 20,
        right: 80,
        bottom: 30,
        left: 50
    },
    width = svg.attr("width") - margin.left - margin.right,
    height = svg.attr("height") - margin.top - margin.bottom,
    g = svg.append("g").attr("transform", "translate(" + margin.left + "," + margin.top + ")");


var xScale = d3.scaleLinear().range([0, width]),
    yScale = d3.scaleLinear().range([height, 0]),
    colorScale = d3.scaleOrdinal(d3.schemeCategory10);


var zoomTransform = d3.zoomIdentity;

var line = d3.line()
    .x(function (d) {
        return zoomTransform.rescaleX(xScale)(d.millis);
    })
    .y(function (d) {
        return yScale(d.reading);
    });
var series, redraw, columns;

d3.tsv("../data/TRACKING.TSV", type, function (error, data) {
    if (error) throw error;

    columns = data.columns.slice(1).map(function (id) {
        return {
            id: id,
            values: data.map(function (d) {
                return {
                    millis: parseInt(d.millis),
                    reading: d[id]
                };
            })
        };
    });

    xScale.domain(d3.extent(data, function (d) {
        return +d.millis;
    }));

    yScale.domain([
    d3.min(columns, function (c) {
            return d3.min(c.values, function (d) {
                return d.reading;
            });
        }),
    d3.max(columns, function (c) {
            return d3.max(c.values, function (d) {
                return d.reading;
            });
        })
  ]);

    var seriesNames = columns.map(function (c) {
        return c.id;
    });

    colorScale.domain(seriesNames);

    var zoom = d3.zoom()
        .scaleExtent([1, 40])
        .translateExtent([[0, -100], [width + 90, height + 100]])
        .on("zoom", zoomed);

    svg.call(zoom);

    xAxis = d3.axisBottom(xScale);
    xAxis.tickFormat(function (val) {
        return (val / 1000) + "s"
    });
    gX = g.append("g")
        .attr("class", "axis axis--x")
        .attr("transform", "translate(0," + height + ")")
        .call(xAxis);


    yAxis = d3.axisLeft(yScale);
    gY = g.append("g")
        .attr("class", "axis axis--y")
        .call(yAxis);
    gY.append("text")
        .attr("transform", "rotate(-90)")
        .attr("y", 6)
        .attr("dy", "0.71em")
        .attr("fill", "#000")
        .text("Reading");

    series = g.selectAll(".series")
        .data(columns)
        .enter().append("g")
        .attr("class", "series");

    series.append("path")
        .attr("class", "line")
        .attr("d", function (d) {
            return line(d.values);
        })
        .style("stroke", function (d) {
            return colorScale(d.id);
        });

    redraw = function () {

        line.x(function (d) {
            return zoomTransform.rescaleX(xScale)(d.millis);
        })
        
        series.select(".line") // change the line
            .attr("d", function (d) {
                return line(d.values);
            });
    }


    // draw legend
    var legend = svg.append("g")
        .attr("class", "legend")
        .attr("transform", "translate(" + width - 40 + ",30)")

    // draw legend colored rectangles
    var legendRows = legend.selectAll(".legend-row")
        .data(seriesNames)
        .enter()
        .append("g")
        .attr("class", "legend-row")
        .attr("transform", function (d, i) {
            return "translate(80, " + (20 + (i * 22)) + ")";
        })

    legendRows
        .append("rect")
        .attr("width", 18)
        .attr("height", 18)
        .style("fill", colorScale)

    legendRows
        .append("text")
        .attr("x", 25)
        .attr("dy", "1.35em")
        .style("font", "10px sans-serif")
        .text(function (d) {
            return d;
        })

});


function type(d, _, columns) {

    for (var i = 1, n = columns.length, c; i < n; ++i) {
        d[c = columns[i]] = +d[c];
    }
    return d;
}

var x_zoom = true;

function zoomed() {

    zoomTransform = d3.event.transform;
    
    gX.call(xAxis.scale(zoomTransform.rescaleX(xScale)));

    redraw();
}