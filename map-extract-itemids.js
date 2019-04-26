/*
 * Example script of using the otbm2json library
 * Changes all tiles on a map to chessboard pattern in global coordinates
 */

const otbm2json = require("./OTBM2JSON-master/otbm2json");

var fs = require('fs'),
    xml2js = require('xml2js');

var parser = new xml2js.Parser();
var name = __dirname + '\\Map.xml';
var youritems = [];
fs.readFile(name, function(err, data) {
    parser.parseString(data, function (err, result) {
//		var myjson = JSON.stringify(result);
        //console.dir(myjson);
        console.log('Done loading, converting...');
		var x=0, y=0;
		var newobj = [];
		var tiles = result.map.tile;
		for(var name in tiles){
			var mytile = tiles[name];
			var myitems = mytile.item;
//			console.log(name+": "+JSON.stringify(mytile));
//			console.log('...contains...');
			for(var thing in myitems) {
				for(var item in myitems[thing]) {
					if(youritems.indexOf(myitems[thing][item].id) == -1)
						youritems.push(myitems[thing][item].id);
				}
			}
		}
		youritems.sort((a, b) => a - b);
		fs.writeFile('mynewmap[items].json', JSON.stringify(youritems, null, 2), 'utf8', function () {});
    });
});
