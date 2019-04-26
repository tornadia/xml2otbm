/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");

var fs = require('fs');

// Read the map data using the otbm2json library
const mapData = otbm2json.read("map/world.otbm");

// Go over all nodes
/*mapData.data.nodes.forEach(function(x) {

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

});*/

fs.writeFile('world.json', JSON.stringify(mapData, null, 2), 'utf8', function () {});
// Write the output to OTBM using the library
//otbm2json.write("chess.otbm", mapData);

