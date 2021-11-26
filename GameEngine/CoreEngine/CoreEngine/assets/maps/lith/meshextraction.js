const fs = require("fs");
const zlib = require('zlib');
const http = require('http');
const https = require('https');

//var levelData = "" + fs.readFileSync('data.lvl');

var importData = "" + fs.readFileSync('export.rbxmx');
var sources = importData.match(/\[CDATA\[[^\]]+\]\]/g);
var sourceBuffer = [];

for (var i = 0; i < sources.length; ++i)
	sourceBuffer[i] = sources[i].match(/\[[^\]\[]+\]/)[0].match(/[^\]\[]+/)[0];

console.log(sources.length);
console.log(sourceBuffer.length);
var levelData = sourceBuffer.join('');
console.log(levelData.length);

fs.writeFile('data.lvl', levelData, function(err) {
	if (err)
		console.log(err);
	else
		console.log('dumped to data.lvl');
});

var ids = [];
var fileIDs = levelData.match(/fileid:[0-9]+/g);

if (fileIDs != null)
{
	for (var i = 0; i < fileIDs.length; ++i)
	{
		var found = false;
		var id = fileIDs[i].match(/[0-9]+/)[0];
		
		for (var j = 0; j < ids.length; ++j)
		{
			if (ids[j] == id)
			{
				found = true;
				
				break;
			}
		}
		
		if (!found)
		{
			console.log('found id: ' + id);
			ids[ids.length] = id;
		}
	}
}

var parser = /\[(-?[0-9]+(\.[0-9]+)?(e-?[0-9]+)?,? ?)+\]/g;
var numberParser = /-?[0-9]+(\.[0-9]+)?(e-?[0-9]+)?/g;

function process(content, id)
{
	console.log("processing " + id);
	
	var vectors = content.match(parser);
	
	if (vectors === null)
		vectors = [];
	
	var outputLines = [];
	var index = 0;
	
	for (var j = 0; j < vectors.length; ++j)
	{
		var axis = vectors[j].match(numberParser);
		var line;
		
		if (j % 3 == 0)
			++index;
		
		switch(j % 3)
		{
		case 0:
			line = 'v ' + axis[0] + ' ' + axis[1] + ' ' + axis[2];
			
			break;
		case 1:
			line = 'vn ' + axis[0] + ' ' + axis[1] + ' ' + axis[2];
			
			break;
		case 2:
			line = 'vt ' + axis[0] + ' ' + axis[1];
		}
		
		outputLines[outputLines.length] = line;
		
		if (j % 9 == 8)
			outputLines[outputLines.length] = 'f ' + (index - 2) + '/' + (index - 2) + '/' + (index - 2) + ' ' + (index - 1) + '/' + (index - 1) + '/' + (index - 1) + ' ' + (index) + '/' + (index) + '/' + (index);
	}
	
	var output = outputLines.join('\n') + '\n';
	
	fs.writeFile('models/' + id + '.obj', output, function(err) {
		if (err)
			console.log(err);
		else
			console.log('dumped to models/' + id + '.obj');
	});
	
	fs.writeFile('models/' + id + '.mesh', content, function(err) {
		if (err)
			console.log(err);
		else
			console.log('dumped to models/' + id + '.mesh');
	});
}

console.log('found ' + ids.length + ' ids');

for (var i = 0; i < ids.length; ++i)
{
	let id = ids[i];
	console.log('fetching asset from "https://assetdelivery.roblox.com/v1/asset/?id=' + id + '"');
	
	https.get("https://assetdelivery.roblox.com/v1/asset/?id=" + id).on('response', (response) => {
		response.setEncoding('utf8');
		console.log(response.statusCode);
		console.log(response.headers.location);
		
		let unzip = zlib.createGunzip();
		let chunks = [];
		
		https.get(response.headers.location).on('response', (response2) => {
			//response2.setEncoding('utf8');
			
			response2.on('data', (chunk2) => {
				chunks.push(chunk2);
			}).on('end', () => {
				let buffer = Buffer.concat(chunks);
				
				if (response2.headers['content-encoding'] == 'gzip')
				{
					zlib.gunzip(buffer, function(err, content) {
						if (err)
							console.log(err);
							
						console.log("unzipped " + id);
						process(content.toString(), id);
					});
				}
				else
					process(buffer.toString(), id);
			});
		});
	});
}