/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");

var fs = require('fs'),
    xml2js = require('xml2js');

const CONVERSION = require("./lib/conversion");

var width, height;
var parser = new xml2js.Parser();
var name = __dirname + '\\Map.xml';
var blocks = [
				[ ] // this can be optimized to allow actual 255x255 block
			];
			
var spawns = "<?xml version=\"1.0\"?>\n<spawns>"
fs.readFile(name, function(err, data) {
    parser.parseString(data, function (err, result) {
        console.log('Done loading, converting...');
		var x=0, y=0, realx=0, realy=0;
		var blockx=0, blocky=0;
		var newobj = [];
		var tiles = result.map.tile;
		width = result.map.$.width;
		height = result.map.$.height;
		for(var name in tiles){
			if(x==256) {
				blocks[blocky][blockx] = newobj;
				newobj = [];
				x = 0;
				blockx++;
				if(blockx==2) {				
					x = 0;
					blockx=0;
					//y+=1;
					blocky++;
					blocks[blocky] = [];
				}
			}
			var mytile = tiles[name];
			
			var myitems = mytile.item;
			var myOtherTele = mytile.ropetele != undefined ? mytile.ropetele : mytile.usetele;
			
			var madeTele = false;
			var madeGate = 0;
			var youritems = {"type":5, "x":x, "y":y, "tileid": null};
			
			var tempobj;
			if(mytile.tele != undefined) {
				tempobj = mytile.tele[0]['$'];
				if(!youritems.items) youritems.items = [];
				youritems.items.push({
						"type":6,
						"id":1387, 
						"destination": {
							"x": tempobj.local_x,
							"y": tempobj.local_y,
							"z": 7
						}
					});
				madeTele = true;
			}
			if(myOtherTele != undefined) {
				tempobj = myOtherTele[0]['$'];
				if(!youritems.items) youritems.items = [];
				youritems.items.push({
						"type":6,
						"id":1387, 
						"destination": {
							"x": tempobj.local_x,
							"y": tempobj.local_y,
							"z": 7
						}
					});
			}
			if(mytile.gate != undefined) {
				madeGate = parseInt(mytile.gate[0]['$'].lvl, 10);
			}
			for(var thing in myitems) {
				for(var item in myitems[thing]) {
					var id = myitems[thing][item].id;
					if(CONVERSION[id]) {
						id = CONVERSION[id];
					}
					
					if(youritems.tileid == null) {
						youritems.tileid = parseInt(id,10);
					} else {
						var doors = {"1219":1227, "1212":1229, "1221":1229};
						if((!madeTele || id != 1387)&&(madeGate == 0 || doors[id] )) {
							if(!youritems.items) youritems.items = [];
							var addItem = {"type":6, "id":parseInt(id,10), "count":1};
							if(madeGate != 0) {
								if(!doors[id]) console.log("Careful, found a level door without traceable id");
								
								addItem.aid = 1000+madeGate;
								addItem.id = parseInt(doors[id]);
								madeGate = 0;
							}
							youritems.items.push(addItem);
						}
					}
				}
			}
			if(mytile.npc) {
				var posx = blockx*256+x;
				spawns += "\n	<spawn centerx=\""+ posx.toString() +"\" centery=\""+blocky.toString()+"\" centerz=\"7\" radius=\"1\">";
				spawns += "\n		<monster name=\""+mytile.npc[0]['$'].name.toLowerCase()+"\" x=\"0\" y=\"0\" z=\"7\" spawntime=\""+mytile.npc[0]['$'].respaw+"\" direction=\"2\" />";
				spawns += "\n	</spawn>";
			}
			
//			var waters = [609,475];
//			if(waters.indexOf(youritems.tileid) == -1)
				newobj.push(youritems);
//			traverse(mytile.item[0]).forEach(function (object) {
//				console.log('...+...');
//			});
			x++;
		}
		console.log('Loaded, check the file.');
		var response = {
			"version": "1.0.1",
			"identifier": 0,
			"data": {
				"type": 0,
				"version": 1,
				"mapWidth": 512, // templepos 337,82
				"mapHeight": 512,
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
		for(var blockY in blocks){
			for(var blockX in blocks[blockY]){
				var myblock = {
					"type":4,
					"x": blockX*256,
					"y": blockY,
					"z": 7,
					"tiles":blocks[blockY][blockX]
				};
				response.data.nodes[0].features.push(myblock);
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
		spawns += "\n</spawns>";
		fs.writeFile('mynewmap-spawn.xml', spawns, 'utf8', function () {});
		fs.writeFile('mynewmap.json', JSON.stringify(response, null, 2), 'utf8', function () {});
		otbm2json.write("mynewmap.otbm", response);
    });
});
