#!/usr/bin/env node
const fs = require('fs');
const PNG = require('pngjs').PNG;
const pixelmatch = require('pixelmatch');

const args = process.argv.slice(2);
if (args.length !== 2) {
    console.log("Pass in 2 images to compare");
    process.exit(1);
}

const img1 = PNG.sync.read(fs.readFileSync(args[0]));
const img2 = PNG.sync.read(fs.readFileSync(args[1]));
const {width, height} = img1;
const diff = new PNG({width, height});

pixelmatch(img1.data, img2.data, diff.data, width, height, {threshold: 0.1});

fs.writeFileSync('diff.png', PNG.sync.write(diff));
console.log("done");
