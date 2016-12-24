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
    zScale = d3.scaleOrdinal(d3.schemeCategory10);

var line = d3.line()
//    .curve(d3.curveBasis)
    .x(function (d) {
        return xScale(d.millis);
    })
    .y(function (d) {
        return yScale(d.reading);
    });

d3.tsv("../data/TRACKING.TSV", type, function (error, data) {
    if (error) throw error;

    var columns = data.columns.slice(1).map(function (id) {
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
    console.log(columns);
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

    zScale.domain(columns.map(function (c) {
        return c.id;
    }));

    var zoom = d3.zoom()
        .scaleExtent([1, 40])
        .translateExtent([[-100, -100], [width + 90, height + 100]])
        .on("zoom", zoomed);

    svg.call(zoom);

    xAxis = d3.axisBottom(xScale);
    xAxis.tickFormat(function(val){
        return (val/1000) + "s"
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
            return zScale(d.id);
        });

    series.append("text")
        .datum(function (d) {
            return {
                id: d.id,
                value: d.values[d.values.length - 1]
            };
        })
        .attr("transform", function (d) {
            return "translate(" + xScale(d.value.millis) + "," + yScale(d.value.reading) + ")";
        })
        .attr("x", 3)
        .attr("dy", "0.35em")
        .style("font", "10px sans-serif")
        .text(function (d) {
            return d.id;
        });
});

function type(d, _, columns) {

    for (var i = 1, n = columns.length, c; i < n; ++i) {
        d[c = columns[i]] = +d[c];
    }
    return d;
}

var x_zoom = true;

function zoomed() {
  if(x_zoom) {
    series.attr("transform", "translate(" + d3.event.transform.x + ",0) scale(" + d3.event.transform.k + ", 1)");
  }
  else if(y_zoom) {
    series.attr("transform", "scale(1, " + d3.event.transform.k + ")");
  }
    
//    gY.call(yAxis.scale(d3.event.transform.rescaleY(yScale)));
    gX.call(xAxis.scale(d3.event.transform.rescaleX(xScale)));
}