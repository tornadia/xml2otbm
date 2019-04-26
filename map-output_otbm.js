/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");

var fs = require('fs');

fs.readFile('world.json',
    // callback function that is called when reading file is done
    function(err, data) { 
        // json data
        var jsonData = data;
 
        // parse json
        var mapData = JSON.parse(jsonData);
		otbm2json.write("world.otbm", mapData);
	}
); 
// Write the output to OTBM using the library

