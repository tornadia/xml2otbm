/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");
/*
const WHITE_TILE = 406;
const BLACK_TILE = 407;

// Read the map data using the otbm2json library
const mapData = otbm2json.read("void.otbm");

// Go over all nodes
mapData.data.nodes.forEach(function(x) {

  x.features.forEach(function(x) {
    
    // Skip anything that is not a tile area
    if(x.type !== otbm2json.HEADERS.OTBM_TILE_AREA) return; 

    // For each tile area; go over all actual tiles
    x.tiles.forEach(function(x) {

      // Skip anything that is not a tile (e.g. house tiles)
      if(x.type !== otbm2json.HEADERS.OTBM_TILE) return; 

      // Create a chessboard pattern using bitwise operators
      // Replace the id property of each tile
      if(x.x & 1 ^ x.y & 1) {
        x.tileid = BLACK_TILE;
      } else {
        x.tileid = WHITE_TILE;
      }

    });

  });

});

// Write the output to OTBM using the library
otbm2json.write("chess.otbm", mapData);
*/

var fs = require('fs'),
    xml2js = require('xml2js');

var width, height;
var parser = new xml2js.Parser();
var name = __dirname + '\\Map.xml';
fs.readFile(name, function(err, data) {
    parser.parseString(data, function (err, result) {
//		var myjson = JSON.stringify(result);
        //console.dir(myjson);
        console.log('Done loading, converting...');
		var x=0, y=0;
		var newobj = [];
		var tiles = result.map.tile;
		width = result.map.$.width;
		height = result.map.$.height;
		for(var name in tiles){
			if(x==width-1) {
				x = 0;
				y+=1;
			}
			var mytile = tiles[name];
			var myitems = mytile.item;
//			console.log(name+": "+JSON.stringify(mytile));
//			console.log('...contains...');
			var youritems = {"type":6, "x":x, "y":y, "tileid": null};
			for(var thing in myitems) {
				for(var item in myitems[thing]) {
					if(youritems.tileid == null) {
						youritems.tileid = parseInt(myitems[thing][item].id,10);
					} else {
						if(!youritems.items) youritems.items = [];
						youritems.items.push({"type":6, "id":parseInt(myitems[thing][item].id,10)});
					}
				}
			}
			var waters = [609,475];
			if(waters.indexOf(youritems.tileid) == -1)
				newobj.push(youritems);
//			traverse(mytile.item[0]).forEach(function (object) {
//				console.log('...+...');
//			});
			x++;
		}
		console.log('Loaded, check the file.');
		var response = {
			"version": "1.0.0",
			"identifier": 0,
			"data": {
				"type": 0,
				"version": 1,
				"mapWidth": parseInt(width,10),
				"mapHeight": parseInt(height,10),
				"itemsMajorVersion": 1,
				"itemsMinorVersion": 3,
				"nodes": [{
					"type": 2,
					"description": "Converted with Tornadia's xml2otbm.",
					"spawnfile": "map-spawn.xml",
					"housefile": "map-house.xml",
					"features": [{
						"type": 4,
						"x": 0, // 337
						"y": 0, // 82
						"z": 7,
						"tiles": newobj
					}]
				}]
			}
		};
		fs.writeFile('mynewmap.json', JSON.stringify(response, null, 2), 'utf8', function () {});
    });
});
