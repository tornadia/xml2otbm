/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");

var fs = require('fs'),
    xml2js = require('xml2js');

var parser = new xml2js.Parser();
var name = __dirname + '\\ctfmap.xml';
var youritems = [];
fs.readFile(name, function(err, data) {
    parser.parseString(data, function (err, result) {
//		var myjson = JSON.stringify(result);
        //console.dir(myjson);
        console.log('Done loading, converting...');
		var x=0, y=0;
		var newobj = [];
		var tiles = result.map.tile;
		for(var tileId in tiles){
			var mytile = tiles[tileId];			
			ground = mytile.$.ground;
			
			if(ground && youritems.indexOf(ground) == -1)
				youritems.push(ground);
			
			var myitems = mytile.item;
			if(!myitems) // prevent null output
				continue;
			
//			console.log(tileId+": "+JSON.stringify(mytile));
//			console.log('...contains...');
			for(var itemId in myitems) {
				var _itemid = myitems[itemId].$.id;
				console.log(myitems[itemId]);
				if(youritems.indexOf(_itemid) == -1)
					youritems.push(_itemid);
			}
			
			// todo impl. portals
		}
		youritems.sort((a, b) => a - b);
		fs.writeFile('mynewmap[items].json', JSON.stringify(youritems, null, 2), 'utf8', function () {});
    });
});
