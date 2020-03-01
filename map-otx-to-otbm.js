/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");
const fs = require('fs');
const xml2js = require('xml2js');
const CONVERSION = require("./lib/conversion_table75");

var map_width, man_height;
var parser = new xml2js.Parser();
var name = __dirname + '\\ctfmap.xml';

// volatile variables (lol i said tile, pun unintended)
var x=0, y=0, z=0, realx=0, realy=0, realz=0;
var blockx=0, blocky=0;
var newobj = [];

// redundant variables
var blocks = []; // blocks[z][y][x]
for (let i = 0; i < 16; i++) { // tibia maps have 16 floors afaik
	blocks[i] = [];
}

// var blocks = [
				// [ ] // this can be optimized to allow actual 255x255 block
			// ];
			
// TODO: recognize PZ zones <tile pz="1"></tile>
// **BIG** README: in OTX we have x, y, and z handles. We didn't back in 7.1 XML's maps
// they also have <tile ground pz>
	
//var spawns = "<?xml version=\"1.0\"?>\n<spawns>"
fs.readFile(name, function(err, data) {
    parser.parseString(data, function (err, result) {
        console.log('Done loading, converting...');
		var tiles = result.map.tile;
		map_width = result.map.$.width;
		map_height = result.map.$.height;
		for(var tileId in tiles){
			var mytile = tiles[tileId];
			x = mytile.$.x;
			y = mytile.$.y;
			z = mytile.$.z;
			
			if(!blocks[z][y]) {
				blocks[z][y] = [];
			}
									// x and y r supposd to be %16 or %256 or %64 or w/e the fuck we using
									
			var ground = tiles[tileId].$.ground;
			if(CONVERSION[ground]) {
				ground = CONVERSION[ground];
			}
			var youritems = {"type":5, x:x%16, y:y%16, "tileid": ground};
			if(mytile.$.pz) {
				youritems.zones = {protection:true}
			}
			
			var myitems = mytile.item;
			for(var item in myitems) {
				var id = myitems[item].$.id;
				if(CONVERSION[id]) {
					id = CONVERSION[id];
				}

				if(!youritems.items) youritems.items = [];
				var addItem = {"type":6, "id":parseInt(id,10), "count":1};
				if(id==1387) {
					addItem.destination = {
							"x": myitems[item].$.destx,
							"y": myitems[item].$.desty,
							"z": myitems[item].$.destz
						}
				}
				
				youritems.items.push(addItem);
			}
			
			blocks[z][y][x] = youritems;
		}
		console.log('Loaded, check the file.');
		var response = {
			"version": "1.0.1",
			"identifier": 0,
			"data": {
				"type": 0,
				"version": 1,
				"mapWidth": map_width, // templepos 337,82
				"mapHeight": map_height,
				"itemsMajorVersion": 1,
				"itemsMinorVersion": 3,
				"nodes": [{
					"type": 2,
					"description": "Converted with Tornadia's xml2otbm.",
					"spawnfile": "mynewmap-spawn.xml",
					"housefile": "mynewmap-house.xml",
					"features": [/*{
						"type": 4,
						"x": 0, //startblock 
						"y": 0, //
						"z": 7,
						"tiles": newobj[0]
					},{
						"type": 4,
						"x": 256, //nextblock
						"y": 0, //
						"z": 7,
						"tiles": newobj[1]
					},{
						"type": 4,
						"x": 0, //
						"y": 256, // etc ..
						"z": 7,
						"tiles": newobj[2]
					}, {
						"type": 12
					}*/]
				}]
			}
		};
		// for(var blockZ in blocks){
			// for(var blockY in blocks[blockZ]){
				// let sBlock = [];
				// for(var blockX in blocks[blockZ][blockY]){
					// // distribute blockX%16
				// }
				// // and here, do 16 blockY. make sure after blockY=map_width, return blockX to 0
				// // perhaps its wiser to do normal iter thru blockspace
				// var myblock = {
					// "type":4,
					// "x": blockX*16,
					// "y": blockY*16,
					// "z": blockZ,
					// "tiles":sBlock
				// };
				// response.data.nodes[0].features.push(myblock);
			// }
		// }
		for(var blockZ in blocks){
			if(blocks[blockZ].length == 0)
				continue;
			
			for(let i = 0; i < Math.ceil(map_height/16); i++) {
				for(let j = 0; j < Math.ceil(map_width/16); j++) {
					personal_block = [];
					for(let I = i*16; I < i*16+16; I++) {
						for(let J = j*16; J < j*16+16; J++) {
							if(blocks[blockZ][J] && blocks[blockZ][J][I])
								personal_block.push(blocks[blockZ][J][I]);
							//else
								//personal_block.push({"type":5, x:J%16, y:I%16, "tileid": 0});
							// else can add water block, void trig or fly tag
							// otherwise we are losing potential coalit malloc
							// might h2reconsider otbm struct?, after all, 256 slot is "optm." in currstruct
							// might prolly prefix to memleak crash overflow (since need malloc 2 proper size) or are it
						}
					}
					var myblock = {
						"type":4,
						"x": i*16, // << here lies the malloc issue tho. we have 16x16 blocks
						"y": j*16,				// with their tiles posit located @ pos%16 for properly
						"z": blockZ,			// can it b dynamic-er? or are it
						"tiles":personal_block
					};
					response.data.nodes[0].features.push(myblock);
				}
			}
		}
		response.data.nodes[0].features.push(
          {
            "type": 12,
            "towns": [
              {
                "type": 13,
                "townid": 1,
                "name": "Survival",
                "x": 336,
                "y": 81,
                "z": 7
              }
            ]
          }
		  );
		//spawns += "\n</spawns>";
		//fs.writeFile('mynewmap-spawn.xml', spawns, 'utf8', function () {});
		fs.writeFile('mynewmap.json', JSON.stringify(response, null, 2), 'utf8', function () {});
		otbm2json.write("mynewmap.otbm", response);
    });
});


			
			// SO ESSENTIALLY you can do 16x16 blocks
			// or process linearly 256 by 256
			
			// so essentially blockx has 256 tile
			// blocky must have 256 blockx
			// and blockz must have 256 blocky
			
			// make sure we are not breaking
			// if(x==256) { // 256 is MAGICNUM memleak break point.
				// blocks[blocky][blockx] = newobj; // put processed objects in previous block
				// newobj = []; // make new space to store process objects
				// x = 0; // start at 0 again
				// blockx++; // new block, keeping in mind blockx*256+x logic
				// if(blockx==map_width/256) { // blockx == 2 (512 map width) ... TODO: make this map_width/256
					// x = 0; // reached enough blocks to fill map width, start at 0
					// blockx=0; // blockx 0 since we're making a new blocky layer
					// //y+=1; // not necessary since calculated per blocky
					// blocky++; // incr. blocky
					// blocks[blocky] = []; // make new blocky space
				// }
			// }
//			var youritems = {"type":5, "x":x, "y":y, "tileid": tiles[tileId].ground};
			// i don't think they need x and y ...